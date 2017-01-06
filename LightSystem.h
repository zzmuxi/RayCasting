#pragma once
#include "LightObject.h"
#include "RayCast.hpp"

#include <algorithm>
#include <cmath>
#include <math.h>
#include <SFML/Graphics.hpp>
#include <chrono>

#define ____PI 3.141592653
#define COSPIBY4 0.25

const long double PI = 3.141592653589793238L;

void Normalize(sf::Vector2f &v) {
  float mag = std::sqrt((v.x * v.x) + (v.y * v.y));
  v.x /= mag;
  v.y /= mag;
}

float Cross2D(const sf::Vector2f &v1, const sf::Vector2f &v2) {
  return ((v1.x * v2.y) - (v1.y * v2.x));
}

float DistanceBetween(const sf::Vector2f &v1, const sf::Vector2f &v2) {
  float x = v1.x - v2.x;
  float y = v1.y - v2.y;

  return (std::sqrt((x * x) + (y * y)));
}

struct Edge {
  sf::Vector2f Start;
  sf::Vector2f End;

  sf::Vector2f FakeStart;
  sf::Vector2f FakeEnd;
};

class Light {
public:
  Light() = default;
  Light(const Light &other)
    : 
    Position(other.Position),
    Radius(other.Radius),
    Color(other.Color),
    Expand(other.Expand)
  {}

  float Attenuation = 0.f;
  sf::Vector2f Position;
  float Radius = 0.f;
  sf::Color Color;
  float Expand = 0.f;

  sf::CircleShape Circle;
};

class LightSystem {
public:


  LightSystem() {
    DrawFont.loadFromFile("ABSTRACT.TTF");
    FrameDelta.setFont(DrawFont);
    FrameDelta.setColor(sf::Color::White);
    FrameDelta.setCharacterSize(8);
    FrameDelta.setPosition({ 20, 700 });

    IterationsAroundCircle.setFont(DrawFont);
    IterationsAroundCircle.setColor(sf::Color::White);
    IterationsAroundCircle.setCharacterSize(8);
    IterationsAroundCircle.setPosition({ 20, 730 });

    SegmentHit.setFont(DrawFont);
    SegmentHit.setColor(sf::Color::White);
    SegmentHit.setCharacterSize(8);
    SegmentHit.setPosition({ 20, 760 });
    SegmentHit.setString("None");

    LastHitSegment.setFont(DrawFont);
    LastHitSegment.setColor(sf::Color::White);
    LastHitSegment.setCharacterSize(8);
    LastHitSegment.setPosition({ 20, 790 });
    LastHitSegment.setString("-1");

    Edges.push_back({});
    Edges.back().Start = { 0, 0 }; Edges.back().End = { 0.f, 800.f };

    Edges.push_back({});
    Edges.back().Start = { 0.f, 800.f }; Edges.back().End = { 800.f, 800.f };

    Edges.push_back({});
    Edges.back().Start = { 800.f, 800.f }; Edges.back().End = { 800.f, 0.f };

    Edges.push_back({});
    Edges.back().Start = { 800.f, 0.f }; Edges.back().End = { 0.f, 0.f };

    Segments.push_back(sf::VertexArray(sf::Lines, 2)); Segments.back()[0].position = { 0.f,   0.f };   Segments.back()[1].position = { 0.f,   800.f };
    Segments.push_back(sf::VertexArray(sf::Lines, 2)); Segments.back()[0].position = { 0.f,   800.f }; Segments.back()[1].position = { 800.f, 800.f };
    Segments.push_back(sf::VertexArray(sf::Lines, 2)); Segments.back()[0].position = { 800.f, 800.f }; Segments.back()[1].position = { 0.f,   800.f };
    Segments.push_back(sf::VertexArray(sf::Lines, 2)); Segments.back()[0].position = { 0.f,   800.f }; Segments.back()[1].position = { 0.f,   0.f };
  }
  ~LightSystem() = default;


  void AddLightObject(const LightObject &obj) {
    Objects.push_back(obj);
  }

  void AddLightObject(const sf::Vector2f &pos, const sf::Vector2f &size, sf::Color color) {
    Objects.push_back({});
    Objects.back().SetPosSize(pos, size);
    Objects.back().BlockingShape.setFillColor(color);

    auto v0 = Objects.back().Vertices[0];
    auto v1 = Objects.back().Vertices[1];
    auto v2 = Objects.back().Vertices[2];
    auto v3 = Objects.back().Vertices[3];

    Segments.push_back(sf::VertexArray(sf::Lines, 2)); Segments.back()[0].position = { pos.x,          pos.y };           Segments.back()[1].position = { pos.x,          pos.y + size.y };
    Segments.push_back(sf::VertexArray(sf::Lines, 2)); Segments.back()[0].position = { pos.x,          pos.y + size.y };  Segments.back()[1].position = { pos.x + size.x, pos.y + size.y };
    Segments.push_back(sf::VertexArray(sf::Lines, 2)); Segments.back()[0].position = { pos.x + size.x, pos.y + size.y };  Segments.back()[1].position = { pos.x + size.x, pos.y };
    Segments.push_back(sf::VertexArray(sf::Lines, 2)); Segments.back()[0].position = { pos.x + size.x, pos.y };           Segments.back()[1].position = { pos.x,          pos.y };

    Edges.push_back({});
    Edges.back().Start     = { pos.x, pos.y };  Edges.back().End    = { pos.x, pos.y + size.y };
    Edges.back().FakeStart = { pos.x, pos.y }; Edges.back().FakeEnd = { pos.x, pos.y + size.y };

    Edges.push_back({}); 
    Edges.back().Start     = { pos.x, pos.y + size.y }; Edges.back().End     = { pos.x + size.x, pos.y + size.y };
    Edges.back().FakeStart = { pos.x, pos.y + size.y }; Edges.back().FakeEnd = { pos.x + size.x, pos.y + size.y  };

    Edges.push_back({}); 
    Edges.back().Start     = { pos.x + size.x, pos.y + size.y }; Edges.back().End     = { pos.x + size.x, pos.y };
    Edges.back().FakeStart = { pos.x + size.x, pos.y + size.y }; Edges.back().FakeEnd = { pos.x + size.x, pos.y };

    Edges.push_back({}); 
    Edges.back().Start     = { pos.x + size.x, pos.y };  Edges.back().End     = { pos.x, pos.y };
    Edges.back().FakeStart = { pos.x + size.x, pos.y };  Edges.back().FakeEnd = { pos.x, pos.y };
  }

  void Render(sf::RenderTarget &tgt, sf::RenderStates &state) {
    static int framecnt = 0;

    IterationsAroundCircle.setString("IterationsAround: " + std::to_string(2 * PI / dtheta));

    tgt.draw(OverallBounds);

    for (auto & obj : Objects)
      obj.Render(tgt);

    for (auto & tri : LitTriangles)
      tgt.draw(tri, state);

    tgt.draw(IterationsAroundCircle);
    tgt.draw(SegmentHit);
    tgt.draw(LastHitSegment);
  }

  void MoveObject(int which, float x_delta, float y_delta)
  {
    Objects[which].BlockingShape.move({ x_delta, y_delta });

    //corner segments start at 4
    int start = ( which + 1 ) * 4;

    Edges[start].End.x += x_delta; Edges[start].Start.x += x_delta;
    Edges[start].End.y += y_delta; Edges[start].Start.y += y_delta;

    Edges[start + 1].End.x += x_delta; Edges[start + 1].Start.x += x_delta;
    Edges[start + 1].End.y += y_delta; Edges[start + 1].Start.y += y_delta;

    Edges[start + 2].End.x += x_delta; Edges[start + 2].Start.x += x_delta;
    Edges[start + 2].End.y += y_delta; Edges[start + 2].Start.y += y_delta;

    Edges[start + 3].End.x += x_delta; Edges[start + 3].Start.x += x_delta;
    Edges[start + 3].End.y += y_delta; Edges[start + 3].Start.y += y_delta;

    LitTriangles.clear();
    AdvanceSweep({ 400.f, 400.f }, 400.f);
  }

  void AddLight(const sf::Vector2f &pt, float atten, sf::Color c) {
    Lights.push_back({});
    Lights.back().Color = c;
    Lights.back().Position = { pt.x, pt.y };
    Lights.back().Radius = atten;
    Lights.back().Circle.setOrigin(atten, atten);
    Lights.back().Circle.setFillColor(sf::Color::Transparent);
    Lights.back().Circle.setRadius(atten);
    Lights.back().Circle.setPosition({ pt.x, pt.y });

    LightTextures.push_back(std::shared_ptr<sf::RenderTexture>(new sf::RenderTexture));
    LightTextures.back()->create(window_size_x, window_size_y);
    DrawLightTexture(LightTextures.size() - 1);
  }

  void AdvanceSweep(sf::Vector2f LightSource, float attenuation)
  {
    static int frame_count = 0;
    static float x_dir = -1, y_dir = 0;
    static int LastHitEdge = -1;
    static sf::Vector2f LAST_SEGMENT_ENDED;
    static sf::Vector2f SEGMENT_STARTED;
    static sf::Vector2f SWEEP_VERY_BEGINNING;

    dtheta = 2 * PI / breaks_around_circle;

    if (theta >= 2 * PI) {
      LitTriangles.clear();
      theta = 0;
      LastHitEdge = -1;
    }

    std::chrono::system_clock::time_point Start = std::chrono::system_clock::now();
    sf::Vector2f Intersection;

    //So we don't have to redraw the light texture, figure out what the offset is from the center of the texture
    //and offset the texture coordinates by that much
    sf::Vector2f CenterOfTexture = sf::Vector2f(LightTexture->getSize().x / 2.f, LightTexture->getSize().y / 2.f);
    sf::Vector2f OffsetFromCenterOfTexture = LightSource - CenterOfTexture;

    sf::Time START;
    while (theta <= 2 * PI) {

      theta += dtheta;
      x_dir = std::sin(theta);
      y_dir = std::cos(theta);

      sf::Vector2f dir{ x_dir, y_dir }; //should not need to normalize

      sf::Vector2f furthest_point = LightSource;
      furthest_point.x += x_dir * 800.f;
      furthest_point.y += y_dir * 800.f;
      int edge_index = -1;

      draw_hit_segment = false;

      if (FindClosestEdge(LightSource, furthest_point, attenuation, Intersection, edge_index)) {

        if (LastHitEdge != edge_index) {      
          if (LastHitEdge != -1) {

            //and the ending point for the last segment will be determined by one final ray cast, exactly in the direction of the point, but at the segment we just left
            sf::Vector2f StoppingPoint = CastRay(LightSource, Intersection, Edges[LastHitEdge].Start, Edges[LastHitEdge].End);
            //now that we have that, we can cap off 

            LitTriangles.push_back(sf::VertexArray(sf::Triangles, 3));
            LitTriangles.back()[0].position = sf::Vector2f({ LightSource.x, LightSource.y });         
            LitTriangles.back()[0].texCoords = sf::Vector2f({ LightSource.x, LightSource.y }) - OffsetFromCenterOfTexture;

            LitTriangles.back()[1].position = sf::Vector2f({ SEGMENT_STARTED.x, SEGMENT_STARTED.y });
            LitTriangles.back()[1].texCoords = sf::Vector2f({ SEGMENT_STARTED.x, SEGMENT_STARTED.y }) - OffsetFromCenterOfTexture;

            LitTriangles.back()[2].position = sf::Vector2f({ StoppingPoint.x, StoppingPoint.y });
            LitTriangles.back()[2].texCoords = sf::Vector2f({ StoppingPoint.x, StoppingPoint.y }) - OffsetFromCenterOfTexture;

            //we need the starting point for this segment to be the point on the edge closest to this intersection
            //for now, we will just use the intersecion point. This will be improved later
            //  IF we sweep around with a high enough precision, this will probably never be noticable
            SEGMENT_STARTED = Intersection;
          }
          else {
            //make sure we start off at the right point
            SEGMENT_STARTED = Intersection;

            //also mark the very beginning so we can get to it when we loop back around
            SWEEP_VERY_BEGINNING = Intersection;
          }

          LastHitEdge = edge_index;
        }
      }
      else {
      }

      LastHitEdge = edge_index;
      //LastHitSegment.setString("LastEdge: " + std::to_string(LastHitEdge));

      DrawSegment[0].position = { LightSource.x, LightSource.y }; DrawSegment[0].color = sf::Color::White;
      DrawSegment[1].position = { Intersection.x, Intersection.y }; DrawSegment[1].color = sf::Color::White;
    }

    std::chrono::system_clock::time_point End = std::chrono::system_clock::now();

    auto delta = End - Start;
    //FrameDelta.setString("SweepTime: " + std::to_string(delta.count()));

    //on our very last iteration, we shouldn't have been able to cap off the last triangle. We need to do so
    LitTriangles.push_back(sf::VertexArray(sf::Triangles, 3));
    LitTriangles.back()[0].position = sf::Vector2f({ LightSource.x, LightSource.y });
    LitTriangles.back()[0].texCoords = sf::Vector2f({ LightSource.x, LightSource.y }) - OffsetFromCenterOfTexture;

    LitTriangles.back()[1].position = sf::Vector2f({ SEGMENT_STARTED.x, SEGMENT_STARTED.y });
    LitTriangles.back()[1].texCoords = sf::Vector2f({ SEGMENT_STARTED.x, SEGMENT_STARTED.y }) - OffsetFromCenterOfTexture;

    LitTriangles.back()[2].position = sf::Vector2f({ SWEEP_VERY_BEGINNING.x, SWEEP_VERY_BEGINNING.y });
    LitTriangles.back()[2].texCoords = sf::Vector2f({ SWEEP_VERY_BEGINNING.x, SWEEP_VERY_BEGINNING.y }) - OffsetFromCenterOfTexture;
    LastHitEdge = -1;
  }


  //start = starting end of the segment, end = the ending part of it, vector = the vector that *might* be between the two endpoints
  bool CanIntersectSegment(sf::Vector2f start, sf::Vector2f end, sf::Vector2f light_source, sf::Vector2f vector)
  {
    sf::Vector2f VecToStart = start - light_source;
    sf::Vector2f VecToEnd = end - light_source;

    Normalize(VecToEnd);
    Normalize(VecToStart);

    float _acrossb = Cross2D(VecToStart, vector);
    float _acrossc = Cross2D(VecToStart, VecToEnd);
    float _ccrossb = Cross2D(VecToEnd, vector);
    float _ccrossa = Cross2D(VecToEnd, VecToStart);

    if ((_acrossb * _acrossc > 0) && (_ccrossb * _ccrossa > 0)) {
      return true;
    }

    return false;
  }

  bool FindClosestEdge(sf::Vector2f LightSource, sf::Vector2f Point, float Attenuation, sf::Vector2f &Intersection, int &edge_index)
  {
    sf::Vector2f VecToPoint;
    sf::Vector2f VecToStart;
    sf::Vector2f VecToEnd;

    sf::Vector2f VecToFakeStart;
    sf::Vector2f VecToFakeEnd;

    float Distance = FLOAT_MAX;
    float MinDistance = FLOAT_MAX;
    bool DidIntersect = false;

    sf::Vector2f ClosestEdge{ FLOAT_MAX, FLOAT_MAX };
    sf::Vector2f MaybeClosestEdge{ FLOAT_MAX, FLOAT_MAX };
    edge_index = -1;
    std::size_t index = 0;
    for (auto & edge : Edges) {

      if (CanIntersectSegment(edge.Start, edge.End, LightSource, Point - LightSource)) {
        MaybeClosestEdge = CastRay(LightSource, Point, edge.Start, edge.End);

        if ((Distance = DistanceBetween(LightSource, MaybeClosestEdge)) < MinDistance) {
          MinDistance = Distance;
          ClosestEdge = MaybeClosestEdge;
          edge_index = index;
          DidIntersect = true;
        }

      }
      index++;
    }

    if (DidIntersect) {
      Intersection = ClosestEdge;
    }

    return DidIntersect;
  }

  void DrawLightTexture(std::size_t which) {
    LightShader.setParameter("color", Lights[which].Color);
    LightShader.setParameter("center", Lights[which].Position);
    LightShader.setParameter("radius", Lights[which].Radius);
    LightShader.setParameter("expand", Lights[which].Expand);
    LightShader.setParameter("windowHeight", window_size_y);

    LightTextures[which]->draw(Lights[which].Circle, &LightShader);
  }


  //data used for visualizing sweeping
  int EdgeToShow = -1;
  sf::Vector2f PointHitByRay;
  sf::VertexArray DrawSegment = sf::VertexArray(sf::Lines, 2);
  sf::VertexArray DrawLine = sf::VertexArray(sf::Lines, 2);
  bool draw_hit_segment = false;
  float theta = 0;
  float dtheta = 2 * PI / 500;
  int frame_delta = 0;
  int breaks_around_circle = 1000;

  sf::Vector2f LastEdgeEnd;
  sf::Vector2f LastEdgePoint;

  std::vector<sf::VertexArray> ObjectTriangles;
  std::vector<sf::VertexArray> BoundsTriangles;

  sf::Text FrameDelta;
  sf::Text IterationsAroundCircle;
  sf::Text SegmentHit;
  sf::Text LastHitSegment;

  sf::Font DrawFont;

  bool advancing = false;
  
  std::vector<Edge> Edges;
  std::vector<sf::VertexArray> LitTriangles;
  sf::VertexArray CurrentTestingRay = sf::VertexArray(sf::Lines, 2);
  std::vector<sf::VertexArray> SpecialCornerRays;

  bool DrawObjectRays = false;
  std::vector<sf::VertexArray> BoundsRays;
  std::vector<sf::VertexArray> ObjectRays;

  sf::RenderTexture *LightTexture = nullptr;

  bool DrawWhiteRays = false;
  std::vector<sf::VertexArray> Segments;

  std::vector<LightObject> Objects;

  std::vector<Light> Lights;
  std::vector<std::shared_ptr<sf::RenderTexture>> LightTextures;
  sf::Shader LightShader;

  sf::RectangleShape OverallBounds;

  unsigned int window_size_y;
  unsigned int window_size_x;
};
