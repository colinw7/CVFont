#ifndef CVFont_H
#define CVFont_H

#include <CPoint2D.h>
#include <vector>

class CVFontShape {
 public:
  enum Type {
    NONE,
    LINE,
    CURVE
  };

 public:
  CVFontShape(const Type &type) :
   type_(type) {
  }

  virtual ~CVFontShape() { }

  const Type &type() const { return type_; }

 private:
  Type type_;
};

//---

class CVFontLine : public CVFontShape {
 public:
  CVFontLine(const CPoint2D &start, const CPoint2D &end) :
   CVFontShape(Type::LINE), start_(start), end_(end) {
  }

  CVFontLine(const CVFontLine &line) :
   CVFontShape(Type::LINE), start_(line.start_), end_(line.end_) {
  }

  const CPoint2D &start() const { return start_; }
  const CPoint2D &end  () const { return end_  ; }

  void setStart(const CPoint2D &p) { start_= p; }
  void setEnd  (const CPoint2D &p) { end_  = p; }

  int startXInd() const { return std::round(start().x*16); }
  int startYInd() const { return std::round(start().y*16); }

  int endXInd() const { return std::round(end().x*16); }
  int endYInd() const { return std::round(end().y*16); }

 private:
  CVFontLine &operator=(const CVFontLine &line);

 private:
  CPoint2D start_;
  CPoint2D end_;
};

//---

class CVFontCurve : public CVFontShape {
 public:
  CVFontCurve(const CPoint2D &p1, const CPoint2D &p2, const CPoint2D &p3, const CPoint2D &p4) :
   CVFontShape(Type::CURVE), p1_(p1), p2_(p2), p3_(p3), p4_(p4) {
  }

  CVFontCurve(const CVFontCurve &curve) :
   CVFontShape(Type::CURVE), p1_(curve.p1_), p2_(curve.p2_), p3_(curve.p3_), p4_(curve.p4_) {
  }

  const CPoint2D &p1() const { return p1_; }
  const CPoint2D &p2() const { return p2_; }
  const CPoint2D &p3() const { return p3_; }
  const CPoint2D &p4() const { return p4_; }

  void setP1(const CPoint2D &p) { p1_ = p; }
  void setP2(const CPoint2D &p) { p2_ = p; }
  void setP3(const CPoint2D &p) { p3_ = p; }
  void setP4(const CPoint2D &p) { p4_ = p; }

  int p1XInd() const { return std::round(p1().x*16); }
  int p1YInd() const { return std::round(p1().y*16); }

  int p2XInd() const { return std::round(p2().x*16); }
  int p2YInd() const { return std::round(p2().y*16); }

  int p3XInd() const { return std::round(p3().x*16); }
  int p3YInd() const { return std::round(p3().y*16); }

  int p4XInd() const { return std::round(p4().x*16); }
  int p4YInd() const { return std::round(p4().y*16); }

 private:
  CVFontCurve &operator=(const CVFontCurve &curve);

 private:
  CPoint2D p1_;
  CPoint2D p2_;
  CPoint2D p3_;
  CPoint2D p4_;
};

//---

class CVFontDef {
 public:
  typedef std::vector<CVFontShape *> Shapes;

 public:
  CVFontDef() { }

  CVFontDef(double width, double ascender, double descender,
            int numLines, CVFontLine *lines, int numCurves, CVFontCurve *curves) :
   width_(width), ascender_(ascender), descender_(descender) {
    assert(width_ + ascender_ + descender_); // mark used

    for (int i = 0; i < numLines; ++i)
      shapes_.push_back(new CVFontLine(lines[i]));

    for (int i = 0; i < numCurves; ++i)
      shapes_.push_back(new CVFontCurve(curves[i]));
  }

  CVFontDef(double width, double ascender, double descender, const std::string &fontStr) :
   width_(width), ascender_(ascender), descender_(descender) {
    if (! parse(fontStr))
      assert(false);
  }

 ~CVFontDef() {
    for (auto &shape : shapes_)
      delete shape;
  }

  const Shapes shapes() const { return shapes_; }

  int addLine(const CPoint2D &p1, const CPoint2D &p2);

  int addCurve(const CPoint2D &p1, const CPoint2D &p2, const CPoint2D &p3, const CPoint2D &p4);

  const CVFontShape &getShape(int i) const;

  void setShape(int i, const CVFontShape &shape);

  void deleteShape(int i);

 private:
  bool parse(const std::string &str);

 private:
  double width_     { 1.0 };
  double ascender_  { 1.0 };
  double descender_ { 0.0 };
  Shapes shapes_;
};

//---

namespace CVFont {
  static const double charWidth   = 1.0;
  static const double charAscent  = 1.0;
  static const double charDescent = 0.5;
  static const double charHeight  = charAscent + charDescent;

  static const double xMargin  = 0.125;
  static const double ySpacing = 0.1;   // ???

  static const double delta = 0.0625;

  const CVFontDef &getFontDef(char c);
  void setFontDef(char c, const CVFontDef &fontDef);

  CPoint2D snapPoint(const CPoint2D &p);

  template<typename LINE_PROC, typename CURVE_PROC>
  void draw(const CVFontDef &font_def, const LINE_PROC &line_proc, const CURVE_PROC &curve_proc) {
    for (const auto &shape : font_def.shapes()) {
      if      (shape->type() == CVFontShape::Type::LINE) {
        const CVFontLine *line = static_cast<const CVFontLine *>(shape);

        line_proc(line->start().x, line->start().y, line->end().x, line->end().y);
      }
      else if (shape->type() == CVFontShape::Type::CURVE) {
        const CVFontCurve *curve = static_cast<CVFontCurve *>(shape);

        curve_proc(curve->p1().x, curve->p1().y, curve->p2().x, curve->p2().y,
                   curve->p3().x, curve->p3().y, curve->p4().x, curve->p4().y);
      }
    }
  }
}

#endif
