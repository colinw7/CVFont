#ifndef CLFont_H
#define CLFont_H

#include <CPoint2D.h>
#include <vector>

class CLFontLine {
 public:
  CLFontLine(const CPoint2D &start, const CPoint2D &end) :
   start_(start), end_(end) {
  }

  const CPoint2D &start() const { return start_; }
  const CPoint2D &end  () const { return end_  ; }

  void setStart(const CPoint2D &p) { start_= p; }
  void setEnd  (const CPoint2D &p) { end_  = p; }

  int startXInd() const { return int(std::round(start().x*16)); }
  int startYInd() const { return int(std::round(start().y*16)); }

  int endXInd() const { return int(std::round(end().x*16)); }
  int endYInd() const { return int(std::round(end().y*16)); }

 private:
  CPoint2D start_;
  CPoint2D end_;
};

//---

class CLFontDef {
 public:
  typedef std::vector<CLFontLine *> Lines;

 public:
  CLFontDef() = default;

  CLFontDef(char c, double width, double ascender, double descender,
            int numLines, CLFontLine *lines);

 ~CLFontDef();

  char c() const { return c_; }

  double width() const { return width_; }

  double ascender () const { return ascender_; }
  double descender() const { return descender_; }

  const Lines &lines() const { return lines_; }

  int addLine(const CPoint2D &p1, const CPoint2D &p2);

  const CLFontLine &getLine(int i) const;

  void setLine(int i, const CLFontLine &line);

  void deleteLine(int i);

 private:
  char   c_         { '\0' };
  double width_     { 1.0 };
  double ascender_  { 1.0 };
  double descender_ { 0.0 };
  Lines  lines_;
};

//---

namespace CLFont {
  static const double charWidth   = 1.0;
  static const double charAscent  = 1.0;
  static const double charDescent = 0.5;
  static const double charHeight  = charAscent + charDescent;

  static const double xMargin  = 0.125;
  static const double ySpacing = 0.1;   // ???

  static const double delta = 0.0625;

  const CLFontDef &getFontDef(char c);
  void setFontDef(char c, const CLFontDef &fontDef);

  CPoint2D snapPoint(const CPoint2D &p);

  template<typename LINE_PROC>
  void draw(const CLFontDef &font_def, const LINE_PROC &line_proc) {
    for (const auto &line : font_def.lines()) {
      line_proc(line->start().x, line->start().y, line->end().x, line->end().y);
    }
  }
}

#endif
