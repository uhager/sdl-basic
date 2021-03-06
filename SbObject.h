/*! \file SbObject.h
  part of SDL2-basic
  author: Ulrike Hager
 */

#ifndef SBOBJECT_H
#define SBOBJECT_H

#include <vector>
#include <iostream>
#include <array>
#include <memory>

#include <SDL2/SDL.h>

#include "SbTimer.h"

class SbTexture;
class SbWindow;

enum class SbHitPosition {
  none, top, bottom, left, right
    };


enum class SbControlDir {
  none, left, right, up, down
};


struct SbRectangle
{
  SbRectangle(double xn, double yn, double wn, double hn)
  : x(xn), y(yn), w(wn), h(hn)
  {}
  SbRectangle() = default;
  double x = 0;
  double y = 0;
  double w = 0;
  double h = 0;
};


struct SbDimension
{
  SbDimension(int wn, int hn)
  : w(wn), h(hn)
  {}
  SbDimension(SDL_Rect rect)
  : w(rect.w), h(rect.h)
  {}
  SbDimension() = default;
  int w = 0;
  int h = 0;
};

  
class SbObject
{
public:
  SbObject() = default;
  SbObject( SbRectangle bounding_box, const SbDimension* ref);
  SbObject( SDL_Rect bounding_rect, const SbDimension* ref);
  
static SbWindow* window;
 
 void center_camera(SDL_Rect& camera, int width, int height) ;
 SbHitPosition check_hit(const SbObject& toHit);
 virtual void handle_event(const SDL_Event& event){}
  virtual int move( );
  virtual void render() ;
  virtual void render(const SDL_Rect &camera);
  virtual void update_size();
  virtual void was_hit();

  SbRectangle bounding_box() { return bounding_box_;};
  SDL_Rect bounding_rect() const {return bounding_rect_;}
  bool has_mouse(){return has_mouse_;}
  bool is_inside(int x, int y);
  void move_bounding_box();
  void move_bounding_rect();
  std::string name(){return name_;}
  std::ostream& print_dimensions(std::ostream& os); 
  void start_timer() {timer_.start();}
  void stop_timer() {timer_.stop();}
  void set_color( int red, int green, int blue );
  Uint32 time() {return timer_.get_time();}
  bool timer_started() { return timer_.started(); }
  int width() const { return bounding_rect_.w;}
  int height() const { return bounding_rect_.h;}
  int pos_x() const { return bounding_rect_.x;}
  int pos_y() const { return bounding_rect_.y;}
  int rel_w() const { return bounding_box_.w;}
  int rel_h() const { return bounding_box_.h;}
  int rel_x() const { return bounding_box_.x;}
  int rel_y() const { return bounding_box_.y;}
  double velocity_x() { return velocity_x_; }
  double velocity_y() { return velocity_y_; }
  
protected:
  const SbDimension* reference_;
  SDL_Rect bounding_rect_ = {70, 200, 20, 70} ;
  //! location and size in terms of window width and height
  SbRectangle bounding_box_ = {0.5, 0.5, 0.05, 0.05} ;
  /*! velocities are in ms to the screen. so smaller is actually faster. Should maybe rename that...
   */
  double velocity_y_ = 0;
  double velocity_x_ = 0;
  double velocity_ = 0;
  std::shared_ptr<SbTexture> texture_ = nullptr;
  SDL_Color color_ = {210, 160, 10, 0};
  SbTimer timer_;
  std::string name_ = "other";
  bool has_mouse_ = false;
  bool render_me_ = true;
};



#endif  // SBOBJECT_H
