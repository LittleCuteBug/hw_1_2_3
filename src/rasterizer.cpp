#include "rasterizer.h"

using namespace std;

namespace CGL {

  RasterizerImp::RasterizerImp(PixelSampleMethod psm, LevelSampleMethod lsm,
    size_t width, size_t height,
    unsigned int sample_rate) {
    this->psm = psm;
    this->lsm = lsm;
    this->width = width;
    this->height = height;
    this->sample_rate = sample_rate;

    sample_buffer.resize(width * height * sample_rate, Color::White);
  }

  // Used by rasterize_point and rasterize_line
  void RasterizerImp::fill_pixel(size_t x, size_t y, Color c) {
    // TODO: Task 2: You might need to this function to fix points and lines (such as the black rectangle border in test4.svg)
    // NOTE: You are not required to implement proper supersampling for points and lines
    // It is sufficient to use the same color for all supersamples of a pixel for points and lines (not triangles)


    int sqrt_sample_rate = sqrt(sample_rate);
    for (int i = 0; i < sqrt_sample_rate; ++i) {
      for (int j = 0; j < sqrt_sample_rate; ++j) {
        sample_buffer[(y*sqrt_sample_rate+j) * width * sqrt_sample_rate + x*sqrt_sample_rate+i] = c;
      }
    }
  }

  // Rasterize a point: simple example to help you start familiarizing
  // yourself with the starter code.
  //
  void RasterizerImp::rasterize_point(float x, float y, Color color) {
    // fill in the nearest pixel
    int sx = (int)floor(x);
    int sy = (int)floor(y);

    // check bounds
    if (sx < 0 || sx >= width) return;
    if (sy < 0 || sy >= height) return;

    fill_pixel(sx, sy, color);
    return;
  }

  // Rasterize a line.
  void RasterizerImp::rasterize_line(float x0, float y0,
    float x1, float y1,
    Color color) {
    if (x0 > x1) {
      swap(x0, x1); swap(y0, y1);
    }

    float pt[] = { x0,y0 };
    float m = (y1 - y0) / (x1 - x0);
    float dpt[] = { 1,m };
    int steep = abs(m) > 1;
    if (steep) {
      dpt[0] = x1 == x0 ? 0 : 1 / abs(m);
      dpt[1] = x1 == x0 ? (y1 - y0) / abs(y1 - y0) : m / abs(m);
    }

    while (floor(pt[0]) <= floor(x1) && abs(pt[1] - y0) <= abs(y1 - y0)) {
      rasterize_point(pt[0], pt[1], color);
      pt[0] += dpt[0]; pt[1] += dpt[1];
    }
  }

  float calA(float xA, float yA, float xB, float yB)
  {
      return yA-yB;
  }

  float calB(float xA, float yA, float xB, float yB)
  {
    return xB-xA;
  }

  float calC(float xA, float yA, float xB, float yB)
  {
    return -xA*(yA-yB) -yA*(xB-xA);
  }

  float checkLine(float a, float b, float c, float x, float y)
  {
    return a*x + b*y + c;
  }

  bool checkInsideTriangle(
      float a0, float b0, float c0,
      float a1, float b1, float c1,
      float a2, float b2, float c2,
      float px, float py) {
    float check0 = checkLine(a0,b0,c0,px,py);
    float check1 = checkLine(a1,b1,c1,px,py);
    float check2 = checkLine(a2,b2,c2,px,py);

    return (
        (check0 >= 0 && check1 >= 0 && check2 >= 0)
        ||
        (check0 <= 0 && check1 <= 0 && check2 <= 0)
        );
  }

  // Rasterize a triangle.
  void RasterizerImp::rasterize_triangle(float x0, float y0,
    float x1, float y1,
    float x2, float y2,
    Color color) {
    // TODO: Task 1: Implement basic triangle rasterization here, no supersampling
    // TODO: Task 2: Update to implement super-sampled rasterization
    float sqrt_sample_rate = floor(sqrt(sample_rate));

    x0*=sqrt_sample_rate;
    x1*=sqrt_sample_rate;
    x2*=sqrt_sample_rate;

    y0*=sqrt_sample_rate;
    y1*=sqrt_sample_rate;
    y2*=sqrt_sample_rate;

    float min_x = min(min(x0,x1),x2)-1;
    float min_y = min(min(y0,y1),y2)-1;
    float max_x = max(max(x0,x1),x2)+1;
    float max_y = max(max(y0,y1),y2)+1;

    //line  (x0,y0) -> (x1,y1)
    float a0 = calA(x0,y0,x1,y1);
    float b0 = calB(x0,y0,x1,y1);
    float c0 = calC(x0,y0,x1,y1);
    float a1 = calA(x1,y1,x2,y2);
    float b1 = calB(x1,y1,x2,y2);
    float c1 = calC(x1,y1,x2,y2);
    float a2 = calA(x2,y2,x0,y0);
    float b2 = calB(x2,y2,x0,y0);
    float c2 = calC(x2,y2,x0,y0);

    int width_supersample = width * sqrt_sample_rate;
    int height_supersample = height * sqrt_sample_rate;

      for (int x = floor(min_x); x < max_x; ++x) {
        for (int y = floor(min_y); y < max_y; ++y) {
          if (x < 0 || x >= width_supersample) continue;
          if (y < 0 || y >= height_supersample) continue;
          float px = float(x) + 0.5;
          float py = float(y) + 0.5;
          if (checkInsideTriangle(a0, b0, c0, a1, b1, c1, a2, b2, c2, px, py)) {
            sample_buffer[y * width_supersample + x] = color;
          }
        }
      }
  }


  void RasterizerImp::rasterize_interpolated_color_triangle(float x0, float y0, Color c0,
    float x1, float y1, Color c1,
    float x2, float y2, Color c2)
  {
    // TODO: Task 4: Rasterize the triangle, calculating barycentric coordinates and using them to interpolate vertex colors across the triangle
    // Hint: You can reuse code from rasterize_triangle



  }


  void RasterizerImp::rasterize_textured_triangle(float x0, float y0, float u0, float v0,
    float x1, float y1, float u1, float v1,
    float x2, float y2, float u2, float v2,
    Texture& tex)
  {
    // TODO: Task 5: Fill in the SampleParams struct and pass it to the tex.sample function.
    // TODO: Task 6: Set the correct barycentric differentials in the SampleParams struct.
    // Hint: You can reuse code from rasterize_triangle/rasterize_interpolated_color_triangle




  }

  void RasterizerImp::set_sample_rate(unsigned int rate) {
    // TODO: Task 2: You may want to update this function for supersampling support

    this->sample_rate = rate;


    this->sample_buffer.resize(width * height * sample_rate, Color::White);
  }


  void RasterizerImp::set_framebuffer_target(unsigned char* rgb_framebuffer,
    size_t width, size_t height)
  {
    // TODO: Task 2: You may want to update this function for supersampling support

    this->width = width;
    this->height = height;
    this->rgb_framebuffer_target = rgb_framebuffer;


    this->sample_buffer.resize(width * height * sample_rate, Color::White);
  }


  void RasterizerImp::clear_buffers() {
    std::fill(rgb_framebuffer_target, rgb_framebuffer_target + 3 * width * height, 255);
    std::fill(sample_buffer.begin(), sample_buffer.end(), Color::White);
  }


  // This function is called at the end of rasterizing all elements of the
  // SVG file.  If you use a supersample buffer to rasterize SVG elements
  // for antialising, you could use this call to fill the target framebuffer
  // pixels from the supersample buffer data.
  //
  void RasterizerImp::resolve_to_framebuffer() {
    // TODO: Task 2: You will likely want to update this function for supersampling support


    for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
        Color col = Color(0,0,0);
        int sqrt_sample_rate = sqrt(sample_rate);
        for (int i = 0; i < sqrt_sample_rate; ++i) {
          for (int j = 0; j < sqrt_sample_rate; ++j) {
            col += sample_buffer[(y*sqrt_sample_rate+j)*width*sqrt_sample_rate+x*sqrt_sample_rate+i];
          }
        }
        for (int k = 0; k < 3; ++k) {
          this->rgb_framebuffer_target[3 * (y * width + x) + k] = (&col.r)[k] * 255/sample_rate;
        }
      }
    }
  }

  Rasterizer::~Rasterizer() { }


}// CGL
