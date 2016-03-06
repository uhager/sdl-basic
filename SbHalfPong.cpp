/* SbHalfPong, sort of like Pong but for one
author: Ulrike Hager
*/

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <random>
#include <algorithm>
#include <functional>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "SbTexture.h"
#include "SbTimer.h"
#include "SbWindow.h"
#include "SbObject.h"

#include "SbHalfPong.h"


/////  globals /////
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

SbWindow* SbObject::window;

struct DeleteFont
{
  void operator()(TTF_Font* font) const {
    if ( font ) {
      TTF_CloseFont( font );
      font = nullptr;
    }
  }
};



/*! Paddle implementation
 */
Paddle::Paddle()
  : SbObject(SCREEN_WIDTH - 70, 200, 20, 80)
{
  //  bounding_rect_ = {}; 
  velocity_y_ = 0;
  velocity_ = 1.0/1200.0;
  SDL_Color color = {210, 160, 10, 0};
  texture_ = std::make_shared<SbTexture>();
  texture_->from_rectangle( window->renderer(), bounding_rect_.w, bounding_rect_.h, color );
  name_ = "paddle";
}



void
Paddle::handle_event(const SDL_Event& event)
{
  
  SbObject::handle_event( event );
  if( event.type == SDL_KEYDOWN && event.key.repeat == 0 ) {
    switch( event.key.keysym.sym ) {
    case SDLK_UP: velocity_y_ -= velocity_; break;
    case SDLK_DOWN: velocity_y_ += velocity_; break;
    }
  }
  else if( event.type == SDL_KEYUP && event.key.repeat == 0 ) {
    switch( event.key.keysym.sym ) {
    case SDLK_UP: velocity_y_ += velocity_; break;
    case SDLK_DOWN: velocity_y_ -= velocity_; break;
    }
  }
  else if (event.type == SDL_MOUSEBUTTONDOWN)
    {
      int mouse_x = -1, mouse_y = -1;
      SDL_GetMouseState( &mouse_x, &mouse_y );
      is_inside( mouse_x, mouse_y );
#ifdef DEBUG
      std::cout << "[Paddle::handle_event] mouse click is " << ( has_mouse()? "inside" : "outside" ) << std::endl;
#endif // DEBUG
    }
  else if (event.type == SDL_MOUSEBUTTONUP) {
    // SDL_GetMouseState( &mouse_x, &mouse_y );
    has_mouse_ = false;
  }
  else if (event.type == SDL_MOUSEMOTION) {
    if ( has_mouse() ) {
      int y_rel = event.motion.yrel;
      bounding_rect_.y += y_rel;
    }
  }
}



int
Paddle::move()
{
  Uint32 deltaT = timer_.get_time();
  int velocity = (int)( window->height() * velocity_y_ * deltaT); 
  bounding_rect_.y += velocity;
  if( ( bounding_rect_.y < 0 ) || ( bounding_rect_.y + bounding_rect_.h > window->height() ) ) {
    bounding_rect_.y -= velocity;
  }
  move_bounding_box();
  timer_.start();
  return 0;
}
    


Spark::Spark(double x, double y, double width, double height)
  : SbObject(x,y,width,height)
{
}



Uint32 
Spark::expire(Uint32 interval, void* param)
{
#ifdef DEBUG
    std::cout << "[Spark::expire] interval " << interval << std::flush;
#endif // DEBUG
  Spark* spark = ((Spark*)param);
#ifdef DEBUG
    std::cout << "[Spark::expire] index " << spark->index_ << std::endl;
#endif // DEBUG
  if (spark) spark->is_dead_ = true;
  return(0);
}





/*! Ball implementation
 */
Ball::Ball()
  : SbObject(50, 300, 25, 25)
{
  velocity_y_ = 1.0/1500.0;
  velocity_x_ = 1.0/1500.0;
  velocity_ = 1.0/1500.0;
  texture_ = std::make_shared<SbTexture>();
  texture_->from_file(window->renderer(), "resources/ball.png", bounding_rect_.w, bounding_rect_.h );
  name_ = "ball";
}



void
Ball::center_in_front(const SDL_Rect& paddleBox)
{
    bounding_rect_.x = paddleBox.x - bounding_rect_.w - 2;
    bounding_rect_.y = paddleBox.y + paddleBox.h / 2 - bounding_rect_.h/2 ;
    move_bounding_box();
}



void
Ball::create_sparks()
{
#ifdef DEBUG
    std::cout << "[Ball::create_sparks]" << std::endl;
#endif // DEBUG
  if (! sparks_.empty() ) {
    sparks_.clear();
  }
  int n_sparks = distr_number(generator_);
  for ( int i = 0 ; i < n_sparks ; ++i ) {
    double x = distr_position(generator_);
    double y = distr_position(generator_);
    double d = distr_size(generator_);
    x += ( bounding_box_.x + bounding_box_.w/2);
    y += ( bounding_box_.y + bounding_box_.h/2);
    Spark toAdd(x, y, d, d);
    toAdd.index_ = i;
    toAdd.set_texture( texture_ );
    toAdd.lifetime_ = Uint32(distr_lifetime(generator_));
    toAdd.timer_.start();
    sparks_.push_back(toAdd);
#ifdef DEBUG
    std::cout << "[Ball::create_sparks] index " << i << " - lifetime " << (sparks_.back()).lifetime() << std::endl;
#endif // DEBUG
    //    std::function<Uint32(Uint32,void*)> funct = std::bind(&Ball::remove_spark, this, std::placeholders::_1, std::placeholders::_2, toAdd.index_ );
    //   sparks_.back().spark_timer_ = SDL_AddTimer(lifetime, Spark::expire, &sparks_.back());
  }
}



void
Ball::delete_spark(int index)
{
  std::remove_if( sparks_.begin(), sparks_.end(),
		  [index](Spark& spark) -> bool { return spark.index() == index;} );
}



int
Ball::move(const SDL_Rect& paddleBox)
{
  int result = 0;
  if ( goal_ ) {
    center_in_front(paddleBox);
    return result;
  }
  Uint32 deltaT = timer_.get_time();
  int x_velocity = (int)( window->width() * velocity_x_ * deltaT );
  int y_velocity = (int)( window->height() * velocity_y_ * deltaT );  
  bounding_rect_.y += y_velocity;
  bounding_rect_.x += x_velocity;
  if ( bounding_rect_.x + bounding_rect_.w >= window->width() ) {
    goal_ = 1;
    center_in_front(paddleBox);
    return goal_;
  }
  
  bool in_xrange = false, in_yrange = false, x_hit = false, y_hit_top = false, y_hit_bottom = false ;
  if ( bounding_rect_.x + bounding_rect_.w/2 >= paddleBox.x &&
       bounding_rect_.x + bounding_rect_.w/2 <= paddleBox.x + paddleBox.w )
    in_xrange = true;
      
  if ( bounding_rect_.y + bounding_rect_.h/2 >= paddleBox.y  &&
       bounding_rect_.y + bounding_rect_.h/2 <= paddleBox.y + paddleBox.h)
     in_yrange = true;

  if ( bounding_rect_.x + bounding_rect_.w >= paddleBox.x               &&
       bounding_rect_.x                   <= paddleBox.x + paddleBox.w )
    x_hit = true;

  if ( bounding_rect_.y + bounding_rect_.h >= paddleBox.y               &&
       bounding_rect_.y                   <= paddleBox.y + paddleBox.h ) {
    if ( bounding_rect_.y > paddleBox.y ) 
      y_hit_bottom = true;
    else if ( bounding_rect_.y + bounding_rect_.h < paddleBox.y + paddleBox.h )
      y_hit_top = true;
  }
  
  if ( ( x_hit && in_yrange )   ) {
    if ( velocity_x_ > 0 ) velocity_x_ *= -1;
    create_sparks();
    result = 2;
  }
  else if (bounding_rect_.x <= 0) {
    if ( velocity_x_ < 0 ) velocity_x_ *= -1;  
  }
  
  if ( ( y_hit_bottom && in_xrange )  || bounding_rect_.y <= 0 ) {
    if ( velocity_y_ < 0 ) velocity_y_ *= -1;
  }
  else if ( ( y_hit_top && in_xrange )|| ( bounding_rect_.y + bounding_rect_.h >= window->height() ) ) {
    if ( velocity_y_ > 0 ) velocity_y_ *= -1;
  }
 
  move_bounding_box();
  timer_.start();
  return result;
}



Uint32
Ball::remove_spark(Uint32 interval, void *param, int index )
{
  ((Ball*)param)->delete_spark(index);
  return(0);
}



void
Ball::render()
{
  SbObject::render();
  if ( sparks_.empty() ) 
    return;
  // std::for_each( sparks_.begin(), sparks_.end(),
  // 		   [](Spark& spark) -> void { if ( !spark.is_dead() ) spark.render(); } ); 
  auto iter = sparks_.begin();
  while ( iter != sparks_.end() ) {
    if ( iter->time() > iter->lifetime() ){
      iter = sparks_.erase(iter);
    }
    else {
      iter->render();
      ++iter;
    }
  }
}


void
Ball::reset()
{
  goal_ = 0;
  if ( velocity_x_ > 0 ) velocity_x_ *= -1;
  timer_.start();
}



Uint32
Ball::resetball(Uint32 interval, void *param )
{
  ((Ball*)param)->reset();
  return(0);
}


/*! GameOver implementation
 */
GameOver::GameOver(std::shared_ptr<TTF_Font> font)
  : SbMessage(0.4,0.55,0.3,0.2)
{
  name_ = "gameover" ;
  font_ = font;
  set_text("Game Over");
}




/*! HighScore implementation
 */
HighScore::HighScore(std::shared_ptr<TTF_Font> font, std::string filename)
  : SbMessage(0.4,0.75,0.5,0.2), savefile_(filename)
{
  font_ = font;
  name_ = "gameover" ;  //!< same name to render only when game over.
}


void
HighScore::new_highscore( int score )
{
#ifdef DEBUG
  std::cout << "[HighScore::new_highscore]" << std::endl;
#endif // DEBUG
  set_text( " ** New Highscore: " + std::to_string(score) + " ** " );
  highscore_ = score;
  write_highscore();
}


void
HighScore::old_highscore( int score )
{
#ifdef DEBUG
  std::cout << "[HighScore::old_highscore]" << std::endl;
#endif // DEBUG
  set_text( "Score: " + std::to_string(score) +  " Highscore: " + std::to_string(highscore_) );
}



int
HighScore::read_highscore()
{
  highscore_ = 0;
  SDL_RWops* file = SDL_RWFromFile( savefile_.c_str() , "rb" );
  if ( !file ) {
    file = SDL_RWFromFile( savefile_.c_str() , "w+b" );
    SDL_RWwrite( file, &highscore_, sizeof(int), 1 );
  }
  else {
    SDL_RWread( file, &highscore_, sizeof(int), 1 );
  }
  SDL_RWclose( file );
  return highscore_;
}



void
HighScore::write_highscore()
{
  SDL_RWops* file = SDL_RWFromFile( savefile_.c_str() , "r+b" );
  if ( !file ) {
    file = SDL_RWFromFile( savefile_.c_str() , "w+b" );
  }
  SDL_RWwrite( file, &highscore_, sizeof(int), 1 );
  SDL_RWclose( file );
}


void run()
{
    SbWindow window("Half-Pong", SCREEN_WIDTH, SCREEN_HEIGHT);
    SbObject::window = &window ;
    std::vector<SbObject*> objects;
    Paddle paddle;
    Ball ball;
    
    std::shared_ptr<TTF_Font> fps_font = std::shared_ptr<TTF_Font>( TTF_OpenFont( "resources/FreeSans.ttf", 120 ), DeleteFont() );
    if ( !fps_font )
      throw std::runtime_error( "TTF_OpenFont: " + std::string( TTF_GetError() ) );

    SbFpsDisplay fps_display( fps_font );
    SbMessage lives(0.2,0.003,0.13,0.07);
    SbMessage score_text(0.5, 0.003, 0.13, 0.07);
    GameOver game_over(fps_font);
    HighScore high_score(fps_font);
    lives.set_font(fps_font);
    score_text.set_font(fps_font);

    int goal_counter = 3;
    lives.set_text( "Lives: " + std::to_string(goal_counter) );
    int score = 0;
    score_text.set_text( "Score: " + std::to_string(score) );

    int highscore = high_score.read_highscore(  );

    SDL_TimerID reset_timer = 0;
    SDL_Event event;
    bool quit = false;

    int frame_counter = 0;

    objects.push_back(&paddle);
    objects.push_back(&ball);
    objects.push_back(&lives);
    objects.push_back(&score_text);
    objects.push_back(&game_over);
    objects.push_back(&high_score);
    objects.push_back(&fps_display);
    
    while (!quit) {
      while( SDL_PollEvent( &event ) ) {
	if (event.type == SDL_QUIT) quit = true;
	else if (event.type == SDL_KEYDOWN ) {
	  switch ( event.key.keysym.sym ) {
	  case SDLK_ESCAPE:
	    quit = true;
	    break;
	  case SDLK_n: case SDLK_SPACE: case SDLK_RETURN:
	    goal_counter = 3;
	    ball.reset();
	    lives.set_text( "Lives: " + std::to_string(goal_counter) );
	    score = 0;
	    score_text.set_text( "Score: " + std::to_string(score) );
	    break;
	  }
	}
	window.handle_event(event);
	std::for_each( objects.begin(), objects.end(),
		       [event] (SbObject* obj) {obj->handle_event( event );} );
      }
      // move objects   
      if ( goal_counter > 0 ) {
	paddle.move();
	int goal = ball.move( paddle.bounding_rect() );
	switch (goal) {
	case 1: 
	  --goal_counter;
	  if (goal_counter > 0 ) {
	    reset_timer = SDL_AddTimer(1000, Ball::resetball, &ball);
	  }
	  else {
	    if ( score > highscore ) {
	      highscore = score;
	      high_score.new_highscore( score );
	    }
	    else {
	      high_score.old_highscore( score ) ;
	    }
	  }
	  lives.set_text( "Lives: " + std::to_string(goal_counter) );
	  break;
	case 2:
	  ++score;
	  score_text.set_text( "Score: " + std::to_string(score) );
	  break;
	}
      }
      else
	 ball.move( paddle.bounding_rect() );

      // fps counter
      fps_display.update();

      // render
      SDL_RenderClear( window.renderer() );
      std::for_each( objects.begin(), objects.end(),
      		     [](SbObject* obj) {if (obj->name() != "gameover") obj->render(); } );

      if ( goal_counter == 0 ) {
	game_over.render();
	high_score.render();
      }
      SDL_RenderPresent( window.renderer() );
      ++frame_counter;  
    }
}


int main()
{
  sdl_init();
  try {
    run();
  }
  catch (const std::exception& expt) {
    std::cerr << expt.what() << std::endl;
  }
  sdl_quit();
  return 0;
}
  
