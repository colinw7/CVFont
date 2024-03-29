#include <CVFont.h>
#include <CVFontDef.h>
#include <CStrParse.h>

bool
CVFontDef::
parse(const std::string &str)
{
  CStrParse parse(str);

  parse.skipSpace();

  while (! parse.eof()) {
    if      (parse.isChar('L')) {
      parse.skipChar();

      double x1, y1, x2, y2;

      parse.skipSpace(); if (! parse.readReal(&x1)) return false;
      parse.skipSpace(); if (! parse.readReal(&y1)) return false;
      parse.skipSpace(); if (! parse.readReal(&x2)) return false;
      parse.skipSpace(); if (! parse.readReal(&y2)) return false;

      addLine(CPoint2D(x1, y1), CPoint2D(x2, y2));
    }
    else if (parse.isChar('C')) {
      parse.skipChar();

      double x1, y1, x2, y2, x3, y3, x4, y4;

      parse.skipSpace(); if (! parse.readReal(&x1)) return false;
      parse.skipSpace(); if (! parse.readReal(&y1)) return false;
      parse.skipSpace(); if (! parse.readReal(&x2)) return false;
      parse.skipSpace(); if (! parse.readReal(&y2)) return false;
      parse.skipSpace(); if (! parse.readReal(&x3)) return false;
      parse.skipSpace(); if (! parse.readReal(&y3)) return false;
      parse.skipSpace(); if (! parse.readReal(&x4)) return false;
      parse.skipSpace(); if (! parse.readReal(&y4)) return false;

      addCurve(CPoint2D(x1, y1), CPoint2D(x2, y2), CPoint2D(x3, y3), CPoint2D(x4, y4));
    }
    else {
      return false;
    }

    parse.skipSpace();
  }

  return true;
}

int
CVFontDef::
addLine(const CPoint2D &p1, const CPoint2D &p2)
{
  CPoint2D sp1 = CVFont::snapPoint(p1);
  CPoint2D sp2 = CVFont::snapPoint(p2);

  shapes_.push_back(new CVFontLine(sp1, sp2));

  return int(shapes_.size() - 1);
}

int
CVFontDef::
addCurve(const CPoint2D &p1, const CPoint2D &p2, const CPoint2D &p3, const CPoint2D &p4)
{
  CPoint2D sp1 = CVFont::snapPoint(p1);
  CPoint2D sp2 = CVFont::snapPoint(p2);
  CPoint2D sp3 = CVFont::snapPoint(p3);
  CPoint2D sp4 = CVFont::snapPoint(p4);

  shapes_.push_back(new CVFontCurve(sp1, sp2, sp3, sp4));

  return int(shapes_.size() - 1);
}

const CVFontShape &
CVFontDef::
getShape(int i) const
{
  return *shapes_[uint(i)];
}

void
CVFontDef::
setShape(int i, const CVFontShape &shape)
{
  *shapes_[uint(i)] = shape;
}

void
CVFontDef::
deleteShape(int i)
{
  delete shapes_[uint(i)];

  for (int j = i + 1; j < int(shapes_.size()); ++j)
    shapes_[uint(j - 1)] = shapes_[uint(j)];

  shapes_.pop_back();
}

//------

const CVFontDef &
CVFont::
getFontDef(char c)
{
  if (c >= ' ' && c <= '~')
    return vfont_defs[c - ' '];
  else
    return vfont_defs[0];
}

void
CVFont::
setFontDef(char c, const CVFontDef &fontDef)
{
  if (c >= ' ' && c <= '~')
    vfont_defs[c - ' '] = fontDef;
}

CPoint2D
CVFont::
snapPoint(const CPoint2D &p)
{
  return CPoint2D(std::round(p.x*16.0)/16.0, std::round(p.y*16.0)/16.0);
}
