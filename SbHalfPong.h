/* SbHalfPong.h
author: Ulrike Hager
*/

#ifndef SBHALFPONG_H
#define SBHALFPONG_H


#include <random>
#include <string>


#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "SbObject.h"
#include "SbMessage.h"


class Ball;
class Spark;
class Paddle;


/////  globals /////
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;




class Paddle : public SbObject
{
public:
  Paddle(const SbDimension* ref);
  Paddle(SDL_Rect rect, const SbDimension* ref);
  void handle_event(const SDL_Event& event);
  int move();
};



class Ball : public SbObject
{
public:
  Ball(const SbDimension* ref);
  /*! \retval 1 if ball in goal
    \retval 0 else
   */
  int move(const SDL_Rect& paddleBox);
  void render();
  /*! Reset after goal.
   */
  void reset();
  static Uint32 resetball(Uint32 interval, void *param );

  
private:
  int goal_ = 0;
  std::vector<Spark> sparks_;
  std::default_random_engine generator_;
  std::uniform_int_distribution<int> distr_number { 15, 30 };
  std::normal_distribution<double> distr_position { 0.0, 0.01 };
  std::normal_distribution<double> distr_size { 0.003, 0.002 };
  std::uniform_int_distribution<int> distr_lifetime { 100, 400 };

  void create_sparks();
  void center_in_front(const SDL_Rect& paddleBox);
  void delete_spark(int index);
};



class Spark : public SbObject
{
  friend class Ball;
public:
  Spark(double x, double y, double width, double height, const SbDimension* ref);
 
  int index() { return index_;}
  bool is_dead() {return is_dead_;}  
  Uint32 lifetime() { return lifetime_;}

private:
  void set_texture(std::shared_ptr<SbTexture> tex) {texture_ = tex;}
  int index_ = 0;
  bool is_dead_ = false;
  Uint32 lifetime_ = 100;
};


  

class GameOver : public SbMessage
{
 public:
  GameOver(std::shared_ptr<TTF_Font> font, const SbDimension* ref);
};



class HalfPong
{
 public:
  HalfPong();
  void move_objects();
  void render( std::vector<SbObject*> objects );
  void run();
  
 private:
  SbWindow window_{"Half-Pong", SCREEN_WIDTH, SCREEN_HEIGHT};
  std::unique_ptr<Ball> ball_;
  std::unique_ptr<Paddle> paddle_;
  //  std::shared_ptr<TTF_Font> font_;
  std::unique_ptr<SbFpsDisplay> fps_display_;
  std::unique_ptr<SbMessage> lives_;
  std::unique_ptr<SbMessage> score_text_;
  std::unique_ptr<GameOver> game_over_;
  std::unique_ptr<SbHighScore> high_score_;
  uint32_t goal_counter_ = 3;
  uint32_t score_ = 0;
};

#endif  // SBHALFPONG_H
