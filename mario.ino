// -*-c-*-
/* 
  mario.ino

  http://ian-albert.com/games/super_mario_bros_maps
*/

#include "makeblock.h"

#define MARIO_TIMER_Y  18
#define MARIO_COINS_Y  16

// https://www.w3schools.com/colors/colors_mixer.asp
#define STAIR_COLOR         0x8B3C04

#define AIR_COLOR           0x5c94fc
#define GROUND_COLOR        0xc84c0c

#define CLOUD_COLOR_1       CRGB::White
#define CLOUD_COLOR_2       0x90adfd
#define CLOUD_COLOR_3       0x7084fc
     
#define WALL_COLOR          0x642606
#define W_QM_COLOR_1        0x8B3C04
#define W_QM_COLOR_2        0xB25303  // mix WALL_COLOR and QMARK_COLOR
#define W_QM_COLOR_3        0xE06E01
#define QMARK_COLOR         0xff8000

#define MARIO_BODY_COLOR    CRGB::Red
#define MARIO_HEAD_COLOR    CRGB::Orange
#define MARIO_FLICKER_COLOR CRGB::White

#define GOOMBA_COLOR        CRGB::Brown

#define COIN_COLOR          CRGB::Yellow
#define MUSHROOM_COLOR      CRGB::Red

#define SUPER_FLICKER_TIME  120       // max 127, mario won't be hurt by enemies during this time

#define MAX_LEVEL_TIME      300*FPS   // 300s == 5min, max 1000 sec

extern const struct mario_levelS level_1_1;

uint32_t mario_hi_score;

void mario_pause() {
  // printf("mario pause\n");
}

void mario_init() {
  LEDS.clear();
  LEDS.setBrightness(config_get_brightness());

  // load hi score from eeprom
  // check if eeprom marker is valid
  if(EEPROM.read(0) == 0x42) {
    EEPROM.get(6, mario_hi_score);
    // mario highscore is max 9999. Everything higher is probably
    // due to previous firmware versions which did not include mario
    // and which didn't care for this memory area
    if(mario_hi_score > 9999)
      mario_hi_score = 0;
  } else {
    mario_hi_score = 0;             // no high score yet
    EEPROM.write(0, 0x42);          // write magic marker
    EEPROM.put(1, 0);               // write (clear) tetris hi score
    EEPROM.put(6, 0);               // write (clear) mario hi score
  }

  // place mario and init its data structures
  game.mario.x.h = 3; game.mario.x.l = 0;
  game.mario.y.h = 7; game.mario.y.l = 0;
  game.mario.scroll = 0;
  game.mario.jump = 0;
  game.mario.jump_press = 0;
  game.mario.speed = 0;
  game.mario.coin.timer = 0;
  game.mario.mushroom.timer = 0;
  game.mario.qmarks_used = 0;
  game.mario.coins = 0;
  game.mario.ending.state = 0;
  game.mario.timer = MAX_LEVEL_TIME;
  game.mario.super = 0;
  game.mario.enemies_activated = 0;
  game.mario.score = 0;

  for(uint8_t e=0;e<MARIO_MAX_ENEMY;e++)
    game.mario.enemy[e].flags = 0;
  
  // init game timer bar
  for(uint8_t x=0;x<15;x++)
    LED(x, MARIO_TIMER_Y) = CRGB::White;
  
  // start level
  game.mario.level = &level_1_1;
}

static void mario_add_coin(void) {
  LED(game.mario.coins, MARIO_COINS_Y) = COIN_COLOR;
  game.mario.coins++;
}

static void mario_dies(void) {
  game.mario.super = -1;       // flicker
  game.mario.ending.state = 8; // end sequence
  game.mario.ending.cnt = 20;  // 20 * (8/60) = ~3 sec
  game.mario.score = 0;
}

static void mario_game_timer(void) {
  if(game.mario.timer) {
    game.mario.timer--;
    
    // still more than 15 seconds left?
    if(game.mario.timer > 15*FPS) {
      // just let the counter run down slowly
      if((game.mario.timer % (MAX_LEVEL_TIME/15)) == 0) 
	LED(game.mario.timer / (MAX_LEVEL_TIME/15), MARIO_TIMER_Y) =
	  CRGB::Black;
      
    } else if(game.mario.timer == 15*FPS) {
      // exactly 15 sec left -> init red "last 15 sec" timer bar
      for(uint8_t x=0;x<15;x++)
	LED(x, MARIO_TIMER_Y) = CRGB::Red;  
    } else
      LED(game.mario.timer/FPS, MARIO_TIMER_Y) = CRGB::Black;
    
    if(!game.mario.timer)
      mario_dies();   // death by timeout
  }
}

static void mario_draw_pixel(uint8_t x, uint8_t y, uint32_t c) {
  if((x >= game.mario.scroll) && (x < game.mario.scroll+15))
    LED(x-game.mario.scroll,y) = c;
}

static uint32_t mario_color_avg(uint32_t ca, uint32_t cb) {
  uint8_t r = ((((ca>>16)&0xff) + ((cb>>16)&0xff)))>>1;
  uint8_t g = ((((ca>>8)&0xff) + ((cb>>8)&0xff)))>>1;
  uint8_t b = ((((ca>>0)&0xff) + ((cb>>0)&0xff)))>>1;

  return ((uint32_t)r<<16) | (r<<8) | (b);
}

uint8_t mario_process(uint8_t keys) {
  const struct mario_levelS *lvl = game.mario.level;

  // ---------------------------------------------------------
  // ----------------------- draw world ----------------------
  // ---------------------------------------------------------
  // subcycle counter for animations like question marks
  game.mario.subcycle++;
  mario_game_timer();
  
  // ----------------------- draw background ----------------------

  for(uint8_t x=0;x<15;x++) {
    game.mario.block[x] = 0x0001;              // assume bottom row is solid
    LED(x,0) = GROUND_COLOR;
    for(uint8_t y=1;y<14;y++)
      LED(x,y) = AIR_COLOR;
  }

  // ----------------------- draw gaps ----------------------
  uint8_t *gaps = (uint8_t*)pgm_read_ptr_near(&lvl->gaps);
  uint8_t gap_x = pgm_read_byte_near(gaps++);
  while(gap_x) {
    // gap makes floor not solid
    if(gap_x >= game.mario.scroll && gap_x < (game.mario.scroll+15)) {
      game.mario.block[gap_x - game.mario.scroll] &= 0xfe;
      mario_draw_pixel(gap_x, 0, AIR_COLOR);
    }

    // check if any enemy is top of this gap and mark it for later
    // processing
    for(uint8_t e=0;e<MARIO_MAX_ENEMY;e++) {
      // process active enemies only
      if(game.mario.enemy[e].flags & 1) {
	if((game.mario.enemy[e].y == 1) &&
	   (game.mario.enemy[e].x == gap_x))
	  game.mario.enemy[e].flags |= 0x40;   // flag
      }
    }
	
    gap_x = pgm_read_byte_near(gaps++);
  }

  // ----------------------- draw clouds ----------------------
  uint8_t (*clouds)[3] = (uint8_t(*)[3])pgm_read_ptr_near(&lvl->clouds);
  uint8_t cloud_x = pgm_read_byte_near(clouds[0]);
  while(cloud_x) {
    uint8_t cloud_y = pgm_read_byte_near(clouds[0]+1);
    uint8_t cloud_l = pgm_read_byte_near(clouds[0]+2);

    // pixel left/right of cloud
    mario_draw_pixel(cloud_x-1, cloud_y, CLOUD_COLOR_2);
    mario_draw_pixel(cloud_x+cloud_l, cloud_y, CLOUD_COLOR_2);
    mario_draw_pixel(cloud_x-1, cloud_y+1, CLOUD_COLOR_3);
    mario_draw_pixel(cloud_x+cloud_l, cloud_y+1, CLOUD_COLOR_3);
    // pixels in cloud
    while(cloud_l) {
      mario_draw_pixel(cloud_x + --cloud_l, cloud_y,   CLOUD_COLOR_1);
      mario_draw_pixel(cloud_x +   cloud_l, cloud_y+1, CLOUD_COLOR_2);
    }
    cloud_x = pgm_read_byte_near((++clouds)[0]);
  }
    
  // ----------------------- draw hills ----------------------
  uint8_t (*hills)[2] = (uint8_t(*)[2])pgm_read_ptr_near(&lvl->hills);
  uint8_t hill_x = pgm_read_byte_near(hills[0]);
  while(hill_x) {
    uint8_t hill_l = pgm_read_byte_near(hills[0]+1);
    uint8_t hill_t = hill_l & 0x80;
    hill_l &= 0x7f;
    
    // pixel left/right of big dark green hills
    if(hill_t) {
      mario_draw_pixel(hill_x-1, 1, 0x00A800);
      mario_draw_pixel(hill_x+hill_l, 1, 0x00A800);
    }
    while(hill_l) {
      hill_l--;
      if(hill_t) {
	// dark hills are two units tall
	mario_draw_pixel(hill_x + hill_l, 1, 0x00A800);
	mario_draw_pixel(hill_x + hill_l, 2, 0x00A800);
      } else
	mario_draw_pixel(hill_x + hill_l, 1, 0x80D010);
    }
    
    hill_x = pgm_read_byte_near((++hills)[0]);
  }
  
  // ----------------------- draw walls ----------------------
  uint8_t (*walls)[3] = (uint8_t(*)[3])pgm_read_ptr_near(&lvl->walls);
  uint8_t wall_x = pgm_read_byte_near(walls[0]);
  while(wall_x) {
    if(wall_x < (game.mario.scroll+15)) {    
      uint8_t wall_y = pgm_read_byte_near(walls[0]+1);
      uint8_t wall_l = pgm_read_byte_near(walls[0]+2);
      while(wall_l) {
	--wall_l;
      
	if(((wall_x + wall_l) >= game.mario.scroll) &&
	   ((wall_x + wall_l) < (game.mario.scroll+15))) {
      
	     // walls are solid
	     game.mario.block[wall_x + wall_l - game.mario.scroll] |=
	       (1<<wall_y);
	     
	     mario_draw_pixel(wall_x + wall_l, wall_y, WALL_COLOR);
	   }
      }
    }
    wall_x = pgm_read_byte_near((++walls)[0]);
  }

  // ----------------------- draw questionmarks ----------------------
  static const uint32_t qmark_colors[] PROGMEM = {
    WALL_COLOR, W_QM_COLOR_1, W_QM_COLOR_2, W_QM_COLOR_3,
    QMARK_COLOR, W_QM_COLOR_3, W_QM_COLOR_2, W_QM_COLOR_1 };

  uint32_t qmark_color =
    pgm_read_dword_near(qmark_colors + ((game.mario.subcycle & 0x38)>>3));
  uint8_t (*qmarks)[2] = (uint8_t(*)[2])pgm_read_ptr_near(&lvl->qmarks);
  uint8_t qmark_x = pgm_read_byte_near(qmarks[0]);
  uint8_t qmark_idx = 0;
  while(qmark_x) {
    if(qmark_x >= game.mario.scroll && qmark_x < (game.mario.scroll+15)) {
      // mask topmost y bits since they are markers for hidden mushrooms
      uint8_t qmark_y = 0x0f & pgm_read_byte_near(qmarks[0]+1);

      // qmarks are solid
      game.mario.block[qmark_x - game.mario.scroll] |= (1<<qmark_y);

      // marked question marks just look like walls
      if(!(game.mario.qmarks_used & (1<<qmark_idx)))
	mario_draw_pixel(qmark_x, qmark_y, qmark_color);
      else
	mario_draw_pixel(qmark_x, qmark_y, WALL_COLOR);
    }
    qmark_x = pgm_read_byte_near((++qmarks)[0]);
    qmark_idx++;
  }

  // ----------------------- draw stairs ----------------------
  uint8_t (*stairs)[2] = (uint8_t(*)[2])pgm_read_ptr_near(&lvl->stairs);
  uint8_t stair_x = pgm_read_byte_near(stairs[0]);
  while(stair_x) {
    if(stair_x < (game.mario.scroll+15)) {    
      uint8_t stair_l = pgm_read_byte_near(stairs[0]+1);
      uint8_t stair_xtra = (stair_l >> 5)&3;
      int8_t stair_dir = (stair_l & 0x80)?-1:1;
      stair_l &= 0x1f;
      if(stair_dir < 0) stair_x += (stair_l-1);
      uint8_t stair_max = stair_l - stair_xtra;

      // ascending stair
      while(stair_l) {
	if((stair_x >= game.mario.scroll) &&
	   (stair_x < (game.mario.scroll+15))) {
      
	  uint8_t h = (stair_l<stair_max)?stair_l:stair_max;
      
	  // stairs are solid
	  game.mario.block[stair_x-game.mario.scroll] |= (0xffff >> (15-h));
	  for(uint8_t y=1;y<=h;y++)
	    mario_draw_pixel(stair_x, y, STAIR_COLOR);
	}
	stair_l--;
	stair_x += stair_dir;
      }
    }
    stair_x = pgm_read_byte_near((++stairs)[0]);
  }
  
  // ----------------------- draw pipes ----------------------
  uint8_t (*pipes)[2] = (uint8_t(*)[2])pgm_read_ptr_near(&lvl->pipes);
  uint8_t pipe_x = pgm_read_byte_near(pipes[0]);
  while(pipe_x) {
    if(pipe_x < (game.mario.scroll+15)) {    
      uint8_t pipe_h = pgm_read_byte_near(pipes[0]+1);

      // left half of pipe
      if((pipe_x >= game.mario.scroll) &&
	 (pipe_x < (game.mario.scroll+15))) {
	game.mario.block[pipe_x - game.mario.scroll] |=
	  0xffff >> (15-pipe_h);
	
	for(uint8_t y=1;y<=pipe_h;y++) {
	  if(y<pipe_h)
	    mario_draw_pixel(pipe_x,   y, 0x00ff00);
	  else
	    mario_draw_pixel(pipe_x,   y, 0x00c000);
	}
      }
      
      // right half of pipe
      if(((pipe_x+1) >= game.mario.scroll) &&
	 ((pipe_x+1) < (game.mario.scroll+15))) {
	game.mario.block[pipe_x + 1 - game.mario.scroll] |=
	  0xffff >> (15-pipe_h);
	
	for(uint8_t y=1;y<=pipe_h;y++) {
	  if(y<pipe_h)
	    mario_draw_pixel(pipe_x+1, y, 0x80ff80);
	  else
	    mario_draw_pixel(pipe_x+1, y, 0x00ff00);
	}
      }
    }
    pipe_x = pgm_read_byte_near((++pipes)[0]);
  }

  // ----------------------- activate enemies ----------------------
  uint8_t (*enemies)[4] = (uint8_t(*)[4])pgm_read_ptr_near(&lvl->enemies);
  uint8_t enemy_x = pgm_read_byte_near(enemies[0]);
  uint8_t enemy_idx = 0;
  while(enemy_x) {
    // check if an not-yet-actvated enemy has entered the screen
    if((!(game.mario.enemies_activated & (1lu<<enemy_idx))) &&
       (enemy_x < (game.mario.scroll+15))) {
      uint8_t enemy_y = pgm_read_byte_near(enemies[0]+1);
      game.mario.enemies_activated |= (1lu<<enemy_idx);

      // use first free slot for enemy
      for(uint8_t e=0;e<MARIO_MAX_ENEMY;e++) {
	if(!(game.mario.enemy[e].flags & 1)) {
	  // activated and moving left
	  game.mario.enemy[e].flags = (enemy_y&0x80)|3 ;
	  game.mario.enemy[e].x = enemy_x;
	  game.mario.enemy[e].y = enemy_y & 0x7f;
	  game.mario.enemy[e].min = pgm_read_byte_near(enemies[0]+2);
	  game.mario.enemy[e].max = pgm_read_byte_near(enemies[0]+3);
	  break;
	}
      }
    }
    enemy_idx++;
    enemy_x = pgm_read_byte_near((++enemies)[0]);
  }
      
  // ----------------------- draw and move enemies ----------------------
  
  for(uint8_t e=0;e<MARIO_MAX_ENEMY;e++) {
    // process active enemies only
    if(game.mario.enemy[e].flags & 1) {
      uint8_t esx = game.mario.enemy[e].x - game.mario.scroll;
      
      // check if enemy has been marked for an offscreen gap
      if(game.mario.enemy[e].flags & 0x40) {
	// offscreen? just remove it
	if(esx > 14) game.mario.enemy[e].flags = 0;
	else {
	  game.mario.enemy[e].y = 0;
	  game.mario.enemy[e].flags &= ~0x40;
	}
      } else if(game.mario.enemy[e].y > 14) {    // .y is unsigned
	// enemy has left the screen vertically
	game.mario.enemy[e].flags = 0;
      } else if(game.mario.enemy[e].max < game.mario.scroll) {
	// enemies max x pos is smaller then scroll position -> we'll never
	// see this enemy again since we cannot scroll left
	game.mario.enemy[e].flags = 0;
      } else {
	if(esx < 15)
	  LED(esx, game.mario.enemy[e].y) = GOOMBA_COLOR;

	if(!(game.mario.subcycle & 0x07)) {
	  // check if enemy falls.
	  // if on-screen check for blocking elements. if offscreen just
	  // fall to ground level as we don't want to maintain collision
	  // informtation offscreen. This should barely be noticable
	  if(game.mario.enemy[e].y == 0)
	    game.mario.enemy[e].y--;
	  if((esx < 15) &&
	     !(game.mario.block[esx] & (1<<(game.mario.enemy[e].y-1))))
	    game.mario.enemy[e].y--;
	  else if((esx >= 15) && (game.mario.enemy[e].y > 1)) {
	    game.mario.enemy[e].y--;
	  } else if(!(game.mario.subcycle & 0x0f)) {
	    // enemy on solid ground, movement half as fast as "falling"
	    if(!(game.mario.enemy[e].flags & 2)) {
	      // moves right
	      if((esx <= 13) &&
		 (!(game.mario.block[esx+1] & (1<<game.mario.enemy[e].y))))
		game.mario.enemy[e].x++;
	      else if((esx > 13) &&
		      (game.mario.enemy[e].x < game.mario.enemy[e].max))
		game.mario.enemy[e].x++;		  
	      else
		game.mario.enemy[e].flags |= 2;  // right -> left
	    } else {
	      // moves left
	      if((esx > 0) && (esx <= 14) &&
		 (!(game.mario.block[esx-1] & (1<<game.mario.enemy[e].y))))
		game.mario.enemy[e].x--;
	      else if(((esx == 0)||(esx > 14)) &&
		      (game.mario.enemy[e].x > game.mario.enemy[e].min))
		game.mario.enemy[e].x--;
	      else
		game.mario.enemy[e].flags &= ~2;  // left -> right
	    }

	    // check if enemy hit mario
	    if((game.mario.enemy[e].x == game.mario.x.h) &&
	       (game.mario.enemy[e].y == game.mario.y.h)) {
	      // printf("mario hit by enemy %d\n", e);

	      // don't hurt mario if he just got hurt, anyways
	      if(!((game.mario.super <  0) &&
		   (game.mario.super > -SUPER_FLICKER_TIME))) {
		// a super mario becomes a normal mario
		if(game.mario.super > 0) game.mario.super = -1;
		else 		         mario_dies();
	      }
		
	      // running into mario actually doesn't kill an enemy ...
	      // game.mario.enemy[e].flags = 0;
	    }
	  }
	}	
      }
    }
  }
  
  // ----------------------- draw coin ----------------------
  if(game.mario.coin.timer) {
    // coin flickers yellow at half frame rate
    if(game.mario.coin.timer & 2)
      LED(game.mario.coin.x - game.mario.scroll, game.mario.coin.y) =
	COIN_COLOR;
      
    game.mario.coin.timer--;
  }

  // ----------------------- draw mushroom ----------------------
  if(game.mario.mushroom.timer) {
    // check if mushroom is still on screen
    if((game.mario.mushroom.y > 14) ||      // y is unsigned!
       (game.mario.mushroom.x < game.mario.scroll) ||
       (game.mario.mushroom.x >= game.mario.scroll+15))
      game.mario.mushroom.timer = 0;
    else {
      // musroom flickers red at half frame rate
      if(game.mario.mushroom.timer & 2)
	LED(game.mario.mushroom.x - game.mario.scroll,
	    game.mario.mushroom.y) = MUSHROOM_COLOR;

      if(!(game.mario.subcycle & 0x07)) {
	// mushroom x pos on screen
	uint8_t msx = game.mario.mushroom.x - game.mario.scroll;
	// check if mushrooms falls
	if(!(game.mario.block[msx] & (1<<(game.mario.mushroom.y-1))))
	  game.mario.mushroom.y--;
	else {
	  // mushroom on solid ground
	  if(!game.mario.mushroom.movement) {
	    // moves right
	    if(!(game.mario.block[msx+1] & (1<<game.mario.mushroom.y)))
	      game.mario.mushroom.x++;
	    else
	      game.mario.mushroom.movement = 1;  // right -> left
	  } else {
	    // moves left
	    if(!(game.mario.block[msx-1] & (1<<game.mario.mushroom.y)))
	      game.mario.mushroom.x--;
	    else
	      game.mario.mushroom.movement = 0;  // left -> right
	  }

	  // check if mushroom hit mario
	  if((game.mario.mushroom.x == game.mario.x.h) &&
	     (game.mario.mushroom.y == game.mario.y.h)) {
	    game.mario.super = 1;
	    game.mario.mushroom.timer = 0;
	  }
	}
      }
      game.mario.mushroom.timer--;
    }
  }
    
  // ----------------------- draw flag ----------------------
  uint8_t flag = pgm_read_byte_near(&lvl->flag);
  if(game.mario.x.h >= flag-8) {
    // draw pole
    mario_draw_pixel(flag, 12, 0x00ff00);
    for(uint8_t y=2;y<12;y++)
      mario_draw_pixel(flag, y, 0xbaffba);

    // flag at top, lowering or at bottom
    if(game.mario.ending.state <= 1)
      mario_draw_pixel(flag-1, 11, 0xbaffba);
    else if(game.mario.ending.state == 2)
      mario_draw_pixel(flag-1, game.mario.ending.cnt, 0xbaffba);
    else
      mario_draw_pixel(flag-1,  2, 0xbaffba);
  }  
  
  // ----------------------- draw castle ----------------------
  uint8_t castle = pgm_read_byte_near(&lvl->castle);
  if(game.mario.x.h >= castle-12) {
    for(uint8_t x=0;x<5;x++) {
      for(uint8_t y=0;y<5;y++) {
	if((y<3)||(x>0 && x<4))
	  mario_draw_pixel(castle+x, y+1, 0x710705);
      }
    }
    // windows and door
    mario_draw_pixel(castle+2, 1, CRGB::Black);
    mario_draw_pixel(castle+2, 2, CRGB::Black);
    mario_draw_pixel(castle+1, 4, CRGB::Black);
    mario_draw_pixel(castle+3, 4, CRGB::Black);

    if(game.mario.ending.state >= 6)
      mario_draw_pixel(castle+2, 6, (game.mario.subcycle&4)?
		       CRGB::Pink:CRGB::White);
  }
    
  // ----------------------- draw mario ----------------------
  // draw mario if he didn't jump out of the screens top
  if(game.mario.y.h < 14) {
    LED(game.mario.x.h-game.mario.scroll, game.mario.y.h) = MARIO_BODY_COLOR;
    if(game.mario.super > 0) {
      LED(game.mario.x.h-game.mario.scroll, game.mario.y.h+1) =
	MARIO_HEAD_COLOR;
      
      if(game.mario.super < SUPER_FLICKER_TIME) {
	// flicker white for 1 sec after becoming "super"
	if(game.mario.super & 0x02) {
	  LED(game.mario.x.h-game.mario.scroll, game.mario.y.h) =
	    MARIO_FLICKER_COLOR;
	  LED(game.mario.x.h-game.mario.scroll, game.mario.y.h+1) =
	    MARIO_FLICKER_COLOR;
	}
	  
	game.mario.super++;
      }
    } else if(game.mario.super < 0) {
      // mario just became normal (again) -> flicker 1 sec
      
      if(game.mario.super > -SUPER_FLICKER_TIME) {
	// flicker white for 1 sec after becoming "normal"
	if(game.mario.super & 0x02)
	  LED(game.mario.x.h-game.mario.scroll, game.mario.y.h) =
	    MARIO_FLICKER_COLOR;
	  
	game.mario.super--;
      }
    }
  }    
    
  // ---------------------------------------------------------
  // -------------------- mario movement ---------------------
  // ---------------------------------------------------------

  // end sequence is running, no user interaction takes place
  if(game.mario.ending.state) {
    char score_str[5];  // max 9999
    
    // score has to be drawn at full rate as it overlays the
    // game screen which is permanently redrawn
    if(game.mario.ending.state == 9) {
      for(uint8_t x=0;x<15;x++) {
	for(uint8_t y=0;y<7;y++) {
	  LED(x,3+y)[0] = LED(x,3+y)[0] >> 2;
	  LED(x,3+y)[1] = LED(x,3+y)[1] >> 2;
	  LED(x,3+y)[2] = LED(x,3+y)[2] >> 2;
	}
      }

      ltoa(game.mario.score, score_str, 10);
      uint8_t l = text_str_len(score_str);
      text_str(score_str, (15-l)/2, 4, 0, 15, CRGB::White);
    }
    
    // limit animation speed to 1/8 frame rate unless we are summing up time ...
    if((!(game.mario.subcycle & 0x07)||
	((game.mario.ending.state == 9)&&(!game.mario.coins)))) {
      switch(game.mario.ending.state) {
	
      case 1:  // mario stuck at pole
	if(game.mario.ending.cnt)
	  game.mario.ending.cnt--;
	else {
	  game.mario.ending.state = 2; // next -> mario sliding down pole
	  game.mario.ending.cnt = 11;
	}
	break;

      case 2:  // mario sliding down pole
	// mario sliding down
	if(game.mario.y.h > 2)
	  game.mario.y.h--;
	
	// flag sliding down
	if(game.mario.ending.cnt > 2)
	  game.mario.ending.cnt--;
	else {
	  game.mario.ending.state = 3; // next -> wait a second
	  game.mario.ending.cnt = 5;
	}
	break;

      case 3:
	if(game.mario.ending.cnt)
	  game.mario.ending.cnt--;
	else
	  game.mario.ending.state = 4; // next -> mario walking to castle
	break;
	
      case 4:
	// mario walks to the castle until he reaches the door
	if(game.mario.x.h < castle + 2) {
	  game.mario.x.h++;
	  // make sure castle scrolls partly into screen
	  if(game.mario.x.h - 10 > game.mario.scroll)
	    game.mario.scroll = game.mario.x.h - 10;
	  // make sure mario walks on ground
	  if(game.mario.x.h > flag)
	    game.mario.y.h = 1;
	} else {
	  game.mario.y.h = 20;         // hide mario
	  game.mario.ending.state = 5; // -> wait another second
	  game.mario.ending.cnt = 5;
	}
	break;
	
      case 5:
	if(game.mario.ending.cnt)
	  game.mario.ending.cnt--;
	else
	  game.mario.ending.state = 6; // -> show flag
	break;

      case 6:
	if(keys & KEY_ROTATE)
	  game.mario.ending.state = 9; // -> count score
	break;	

      case 7:
	if(!keys)
	  return 1;                    // -> end game
	break;

	// just end (mario fell into gap or was killed ...)
      case 8:
	if(game.mario.ending.cnt) {
	  game.mario.ending.cnt--;

	  // mario rises up and and drops down when he dies
	  if(game.mario.ending.cnt > 15) game.mario.y.h++;
	  else	                         game.mario.y.h--;
	    
	} else

	  // game.mario.ending.state = 9; // TODO: test only
	  game.mario.ending.state = 7; // -> wait for all key to be unpressed
	break;

	// prepare for score counting
      case 9:
	// add coins to score
	if(game.mario.coins) {
	  game.mario.score += 100;

	  // remove coin from counter ...
	  game.mario.coins--;
	  // ... and from screen
	  LED(game.mario.coins, MARIO_COINS_Y) = CRGB::Black;
	} else if(game.mario.timer > 0) {
	  if(game.mario.timer > FPS) {
	    // every second left is worth one pt
	    game.mario.score++;
	    game.mario.timer -= FPS;
	  } else
	    game.mario.timer = 0;

	  LED(game.mario.timer / (MAX_LEVEL_TIME/15), MARIO_TIMER_Y) =
	    CRGB::Black;      
	} else {
	  // wait two seconds
	  game.mario.ending.cnt = 10;
	  game.mario.ending.state = 10;

	  if(game.mario.score > mario_hi_score)
	    EEPROM.put(6, game.mario.score); // write new high score
	}
	break;

      case 10:
	// wait some time then wait for keys to be released
	if(game.mario.ending.cnt) {
	  game.mario.ending.cnt--;
	} else
	  game.mario.ending.state = 7;
	break;	
      }
      
      
    }
  } else {
    // mario x pos on screen. Used to address the "block" array
    uint8_t msx = game.mario.x.h - game.mario.scroll;
    
    // check if mario is still jumping. Ignore any other movement
    // requests then
    if(game.mario.jump) {
      if((keys & KEY_ROTATE) && (game.mario.jump_press)) {
	// max 14 boosts
	if(game.mario.jump_press < 13) {
	  game.mario.jump_press++;
	  game.mario.jump++;
	}
      } else
	game.mario.jump_press=0;
      
      // check if new position is not blocked. Super mario is one block taller
      if((!(game.mario.block[msx] & (1<<(game.mario.y.h+1)))) &&
	 ((game.mario.super <= 0)||
	  (!(game.mario.block[msx] & (1<<(game.mario.y.h+2)))))) {
	
	// stay "still in the air" for one frame at the end of jump
	if(game.mario.jump > 4)
	  // effectively causing mario to move every 4th frame
	  game.mario.y.w += 64;
	
	game.mario.jump--;
      } else {
	// user hit something from underneath. check if it was a questionmark	
	uint8_t (*qmarks)[2] = (uint8_t(*)[2])pgm_read_ptr_near(&lvl->qmarks);
	uint8_t qmark_x = pgm_read_byte_near(qmarks[0]);
	uint8_t qmark_idx = 0;
	while(qmark_x) {
	  if(!(game.mario.qmarks_used & (1<<qmark_idx))) {
	    if(qmark_x == game.mario.x.h) {
	      uint8_t hit_y = (game.mario.super>0)?game.mario.y.h+2:
		game.mario.y.h+1;
	      uint8_t qmark_y = pgm_read_byte_near(qmarks[0]+1);
	      if((0x0f & qmark_y) == hit_y) {
		game.mario.qmarks_used |= (1lu<<qmark_idx);

		if(!(0xf0 & qmark_y)) {
		  // ordinary coin
		  mario_add_coin();
		  game.mario.coin.x = qmark_x;
		  game.mario.coin.y = qmark_y+1;
		  game.mario.coin.timer = 60;
		} else if((0xf0 & qmark_y) == 0x80) {
		  // mushroom
		  game.mario.mushroom.x = qmark_x;
		  game.mario.mushroom.y = (0x0f & qmark_y)+1;
		  game.mario.mushroom.timer = 255;
		  game.mario.mushroom.movement = 0;   // right
		}
	      }
	    }
	  }
	  
	  qmark_x = pgm_read_byte_near((++qmarks)[0]);
	  qmark_idx++;
	}
	
	game.mario.jump = 0;
      }
      
    } else {
      // user wants to jump and he stands on solid ground
      if(keys & KEY_ROTATE) {
	if((game.mario.block[msx] & (1<<(game.mario.y.h-1))) &&
	   !game.mario.jump_press) {
	  game.mario.jump = 8;
	  game.mario.jump_press = 1;
	}
      } else
	game.mario.jump_press = 0;
      
      // mario falls down whenever there's no solid ground underneath
      if((game.mario.y.h > 0) &&
	 (!(game.mario.block[msx] & (1<<(game.mario.y.h-1))))) {
	game.mario.y.w -= 64;

	for(uint8_t e=0;e<MARIO_MAX_ENEMY;e++) {
	// process active enemies only
	    if(game.mario.enemy[e].flags & 1) {
	    // check if mario hit enemy
	    if((game.mario.enemy[e].x == game.mario.x.h) &&
	       (game.mario.enemy[e].y == game.mario.y.h)) {
	      // printf("mario fell/jumped onto enemy %d\n", e);
	      game.mario.score += 100;  // killing enemy is worth 100 pts
	      game.mario.enemy[e].flags = 0;
	    }
	  }
	}
      }
      
      // mario fell in a gap -> game ends
      if(game.mario.y.h == 0)
	mario_dies();
    }
    
    // accelerate and decellerate mario
    if(keys & KEY_RIGHT) {
      // max speed 24 -> 60/(256/24) blocks per second
      if(game.mario.speed <  24)
	game.mario.speed += 3;
    } else if(keys & KEY_LEFT) {
      if(game.mario.speed > -24)
	game.mario.speed -= 3;
    } else {
      if(game.mario.speed > 0) game.mario.speed--;
      if(game.mario.speed < 0) game.mario.speed++;
    }
    
    // player wants to move right
    if((game.mario.speed > 0) && (game.mario.x.h < 255))
      // no obstacle right
      if((!(game.mario.block[msx+1] & (1<<game.mario.y.h))) &&
	 ((game.mario.super <= 0) || // also not on sm "head"
	  (!(game.mario.block[msx+1] & (1<<game.mario.y.h+1)))))
	game.mario.x.w += game.mario.speed;
    
    // player wants to move left
    if((game.mario.speed < 0) && (game.mario.x.h > 0))
      // no obstacle left
      if((!(game.mario.block[msx-1] & (1<<game.mario.y.h))) && 
	 ((game.mario.super <= 0) || // also not on sm "head"
	  (!(game.mario.block[msx-1] & (1<<game.mario.y.h+1)))))
	 game.mario.x.w += game.mario.speed;
    
    // make sure mario cannot leave the screen to the left
    if(game.mario.x.h < game.mario.scroll) {
      game.mario.x.h = game.mario.scroll;
      game.mario.x.l = 0;
    }
    
    // make world scroll if mario would be drawn right
    // of screens center
    if(game.mario.x.h - 7 > game.mario.scroll)
      game.mario.scroll = game.mario.x.h - 7;

    // check if mario has reached the flag
    if(game.mario.x.h == flag) {
      game.mario.ending.state = 1;  // state 1->stuck to flag pole
      game.mario.ending.cnt = 5;
    }

    // check if mario hit mushroom
    if(game.mario.mushroom.timer &&
       (game.mario.mushroom.x == game.mario.x.h) &&
       (game.mario.mushroom.y == game.mario.y.h)) {
	game.mario.score += 100;  // becoming super mario is worth 100 pts
	game.mario.super = 1;
	game.mario.mushroom.timer = 0;
    }

    for(uint8_t e=0;e<MARIO_MAX_ENEMY;e++) {
      // process active enemies only
      if(game.mario.enemy[e].flags & 1) {
	// check if mario hit enemy
	if((game.mario.enemy[e].x == game.mario.x.h) &&
	   (game.mario.enemy[e].y == game.mario.y.h)) {
	  // printf("mario ran into enemy %d\n", e);

	  // don't get hurt if still in the "just got hit" phase
	  if(!((game.mario.super < 0) &&
	       (game.mario.super > -SUPER_FLICKER_TIME))) {
	    // a super mario becomes a normal mario
	    if(game.mario.super > 0) game.mario.super = -1;
	    else 		     mario_dies();
	  }
	  
	  // being run over by mario actually doesn't kill an enemy ...
	  // game.mario.enemy[e].flags = 0;
	}
      }
    }
  }
  
  return 0;
}
