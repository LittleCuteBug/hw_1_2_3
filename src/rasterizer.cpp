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


    int scale_factor = sqrt(sample_rate);
    if(x<0||x>=width*scale_factor) return;
    if(y<0||y>=height*scale_factor) return;
    sample_buffer[y * width * scale_factor + x] = c;
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

    int scale_factor = sqrt(sample_rate);
    for (int i = 0; i < scale_factor; ++i) {
      for (int j = 0; j < scale_factor; ++j) {
        fill_pixel(sx*scale_factor+i, sy*scale_factor+j, color);
      }
    }
    return;
  }

  vector<pair<int,int>> draw_line(float _x0, float _y0, float _x1, float _y1) {
    vector<pair<int,int>> points;
    if (_x0 > _x1) {
      swap(_x0, _x1); swap(_y0, _y1);
    }

    int x0 = floor(_x0);
    int y0 = floor(_y0);
    int x1 = floor(_x1);
    int y1 = floor(_y1);

    int a = x1-x0;
    int b = y1-y0;

    int x = 0;
    int y = 0;

    bool neg_y = (b<0);

    if(neg_y) b = -b;

    bool is_swap = a<b;

    if(is_swap)
      swap(a,b);

    int d = 2*b-a;
    while (true)
    {
      //draw (x,y)
      int dx = x;
      int dy = y;
      if(is_swap)
        swap(dx,dy);
      if(neg_y)
        dy = -dy;
      points.push_back(pair<int,int> (x0+dx, y0+dy));

      if (x==a) break;
      if (d>0) {
        y++;
        d-=2*a;
      }
      x++;
      d+=2*b;
    }
    return points;
  }

  // Rasterize a line.
  void RasterizerImp::rasterize_line(float x0, float y0,
    float x1, float y1,
    Color color) {
    vector<pair<int,int>> points = draw_line(x0+0.5f,y0+0.5f,x1+0.5f,y1+0.5f);
    for( auto point : points) {
      rasterize_point(point.first, point.second, color);
    }
  }

  bool inside_triangle(float x, float y, float x0, float y0, float x1, float y1, float x2, float y2){

    Vector2D AB = Vector2D(x1-x0, y1-y0);
    Vector2D BC = Vector2D(x2-x1, y2-y1);
    Vector2D CA = Vector2D(x0-x2, y0-y2);
    Vector2D AX = Vector2D(x-x0,y-y0);
    Vector2D BX = Vector2D(x-x1,y-y1);
    Vector2D CX = Vector2D(x-x2,y-y2);

    float a = cross(AB,AX);
    float b = cross(BC,BX);
    float c = cross(CA,CX);

    return ((a>=0&&b>=0&&c>=0)||(a<=0&&b<=0&&c<=0));
  }

  vector<pair<int,int>> draw_triangle(float x0, float y0, float x1, float y1, float x2, float y2) {
    vector<pair<int,int>> points;
    if(x0<x1){
      swap(x0,x1);
      swap(y0,y1);
    }

    if(x0<x2){
      swap(x0,x2);
      swap(y0,y2);
    }

    if(x1>x2){
      swap(x1,x2);
      swap(y1,y2);
    }

    // x0 > x2 > x1
    vector<pair<int,int>> AB = draw_line(x0,y0,x1,y1);
    vector<pair<int,int>> BC = draw_line(x1,y1,x2,y2);
    vector<pair<int,int>> CA = draw_line(x0,y0,x2,y2);

    int i = 0;
    int j = 0;
    int x_start = min(AB[0].first,BC[0].first);
    int x = x_start;
    int min_y = INT_MAX;
    int max_y = INT_MIN;
    while (i<AB.size() && j<BC.size())
    {
      while (i<AB.size() && AB[i].first == x) {
        min_y = min(AB[i].second, min_y);
        max_y = max(AB[i].second, max_y);
        i++;
      }
      while (j<BC.size() && BC[j].first == x) {
        min_y = min(BC[j].second, min_y);
        max_y = max(BC[j].second, max_y);
        j++;
      }
      if(min_y != INT_MAX && max_y != INT_MIN) {
        for (int y = min_y+1; y < max_y; ++y) {
          if(x!= x_start || inside_triangle(x + 0.5f, y + 0.5f, x0, y0, x1, y1, x2, y2))
            points.push_back(pair<int, int>(x, y));
        }
        vector<int> arr = {min_y-1, min_y, max_y, max_y+1};
        for (int y : arr) {
          if (inside_triangle(x + 0.5f, y + 0.5f, x0, y0, x1, y1, x2, y2))
            points.push_back(pair<int, int>(x, y));
        }
      }
      x++;
      min_y = INT_MAX;
      max_y = INT_MIN;
    }

    i = AB.size()-1;
    j = CA.size()-1;
    x_start = max(AB[AB.size()-1].first,CA[CA.size()-1].first);
    x = x_start;
    min_y = INT_MAX;
    max_y = INT_MIN;
    while (i>=0 && j>=0)
    {
      while (i>=0 && AB[i].first == x) {
        min_y = min(AB[i].second, min_y);
        max_y = max(AB[i].second, max_y);
        i--;
      }
      while (j>=0 && CA[j].first == x) {
        min_y = min(CA[j].second, min_y);
        max_y = max(CA[j].second, max_y);
        j--;
      }
      if(min_y != INT_MAX && max_y != INT_MIN) {
        for (int y = min_y+1; y < max_y; ++y) {
          if(x!= x_start || inside_triangle(x + 0.5f, y + 0.5f, x0, y0, x1, y1, x2, y2))
            points.push_back(pair<int, int>(x, y));
        }
        vector<int> arr = {min_y-1, min_y, max_y, max_y+1};
        for (int y : arr) {
          if (inside_triangle(x + 0.5f, y + 0.5f, x0, y0, x1, y1, x2, y2))
            points.push_back(pair<int, int>(x, y));
        }
      }
      x--;
      min_y = INT_MAX;
      max_y = INT_MIN;
    }
    return points;
  }

  // Rasterize a triangle.
  void RasterizerImp::rasterize_triangle(float x0, float y0,
    float x1, float y1,
    float x2, float y2,
    Color color) {
    // TODO: Task 1: Implement basic triangle rasterization here, no supersampling
    // TODO: Task 2: Update to implement super-sampled rasterization
    float scale_factor = floor(sqrt(sample_rate));

    x0=(x0+0.5f)*scale_factor;
    x1=(x1+0.5f)*scale_factor;
    x2=(x2+0.5f)*scale_factor;

    y0=(y0+0.5f)*scale_factor;
    y1=(y1+0.5f)*scale_factor;
    y2=(y2+0.5f)*scale_factor;

    vector<pair<int,int>> points = draw_triangle(x0,y0,x1,y1,x2,y2);
    for( auto point : points) {
      fill_pixel(point.first, point.second, color);
    }

  }

  float cal_area(float x0, float y0, float x1, float y1, float x2, float y2) {
    Matrix3x3 matrix3X3 = Matrix3x3(x0,y0,1,x1,y1,1,x2,y2,1);
    return 0.5f * float(matrix3X3.det());
  }

  Color cal_color(float x, float y, float x0, float y0, Color c0, float x1, float y1, Color c1, float x2, float y2, Color c2) {
    float S = cal_area(x0,y0,x1,y1,x2,y2);
    float a = cal_area(x,y,x1,y1,x2,y2);
    float b = cal_area(x0,y0,x,y,x2,y2);
    float c = cal_area(x0,y0,x1,y1,x,y);
    return c0*(a/S) + c1*(b/S) + c2*(c/S);
  }

  void RasterizerImp::rasterize_interpolated_color_triangle(float x0, float y0, Color c0,
    float x1, float y1, Color c1,
    float x2, float y2, Color c2)
  {
    // TODO: Task 4: Rasterize the triangle, calculating barycentric coordinates and using them to interpolate vertex colors across the triangle
    // Hint: You can reuse code from rasterize_triangle
    float scale_factor = floor(sqrt(sample_rate));

    x0=(x0+0.5f)*scale_factor;
    x1=(x1+0.5f)*scale_factor;
    x2=(x2+0.5f)*scale_factor;

    y0=(y0+0.5f)*scale_factor;
    y1=(y1+0.5f)*scale_factor;
    y2=(y2+0.5f)*scale_factor;

    vector<pair<int,int>> points = draw_triangle(x0,y0,x1,y1,x2,y2);
    for( auto point : points) {
      Color color = cal_color(point.first, point.second, x0, y0, c0, x1, y1, c1, x2, y2, c2);
      fill_pixel(point.first, point.second, color);
    }
  }

  Vector2D transfer_point(float x, float y, float x0, float y0, float x1, float y1, float x2, float y2,
                          float u0, float v0, float u1, float v1, float u2, float v2) {
    float S = cal_area(x0,y0,x1,y1,x2,y2);
    float a = cal_area(x,y,x1,y1,x2,y2);
    float b = cal_area(x0,y0,x,y,x2,y2);
    float c = cal_area(x0,y0,x1,y1,x,y);
    return Vector2D(u0,v0)*(a/S) + Vector2D(u1,v1)*(b/S) + Vector2D(u2,v2)*(c/S);
  }

  void RasterizerImp::rasterize_textured_triangle(float x0, float y0, float u0, float v0,
    float x1, float y1, float u1, float v1,
    float x2, float y2, float u2, float v2,
    Texture& tex)
  {
    // TODO: Task 5: Fill in the SampleParams struct and pass it to the tex.sample function.
    // TODO: Task 6: Set the correct barycentric differentials in the SampleParams struct.
    // Hint: You can reuse code from rasterize_triangle/rasterize_interpolated_color_triangle

    float scale_factor = floor(sqrt(sample_rate));

    x0=(x0+0.5f)*scale_factor;
    x1=(x1+0.5f)*scale_factor;
    x2=(x2+0.5f)*scale_factor;

    y0=(y0+0.5f)*scale_factor;
    y1=(y1+0.5f)*scale_factor;
    y2=(y2+0.5f)*scale_factor;

    vector<pair<int,int>> points = draw_triangle(x0,y0,x1,y1,x2,y2);
    for( auto point : points) {
      Vector2D p_uv = transfer_point(point.first + 0.5f, point.second + 0.5f,x0,y0,x1,y1,x2,y2,u0,v0,u1,v1,u2,v2);
//      Vector2D p_dx_uv = transfer_point(point.first + 1.5f, point.second + 0.5f, x0, y0, x1, y1, x2, y2, u0, v0, u1, v1, u2, v2);
//      Vector2D p_dy_uv = transfer_point(point.first + 0.5f, point.second + 1.5f, x0, y0, x1, y1, x2, y2, u0, v0, u1, v1, u2, v2);
//      SampleParams sampleParams;
//      sampleParams.p_uv = p_uv;
//      sampleParams.psm = psm;
//      sampleParams.lsm = lsm;

//      cout<< point.first + 0.5f << " " << point.second + 0.5f << " " << x0 << " " << y0 << " " << x1 << " " << y1 << " " << x2 << " " << y2 <<endl;
//      cout<< p_uv.x << " " << p_uv.y << " " << u0 << " " << v0 << " " << u1 << " " << v1 << " " << u2 << " " << v2 <<endl;
//      cout<< endl;
      Color color;
      if (psm == 0)
        color = tex.sample_nearest(p_uv,0);
      else
        color = tex.sample_bilinear(p_uv,0);
      fill_pixel(point.first, point.second, color);
    }
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

    int scale_factor = sqrt(sample_rate);
    for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
        Color col = Color(0,0,0);
        for (int i = 0; i < scale_factor; ++i) {
          for (int j = 0; j < scale_factor; ++j) {
            col += sample_buffer[(y * scale_factor + j) * width * scale_factor + x * scale_factor + i];
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
