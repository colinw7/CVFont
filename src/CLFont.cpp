#include <CLFont.h>
#include <CLFontDef.h>

CLFontDef::
CLFontDef(char c, double width, double ascender, double descender,
          int numLines, CLFontLine *lines) :
 c_(c), width_(width), ascender_(ascender), descender_(descender)
{
  for (int i = 0; i < numLines; ++i)
    lines_.push_back(new CLFontLine(lines[i]));
}

CLFontDef::
~CLFontDef()
{
  for (auto &line : lines_)
    delete line;
}

int
CLFontDef::
addLine(const CPoint2D &p1, const CPoint2D &p2)
{
  CPoint2D sp1 = CLFont::snapPoint(p1);
  CPoint2D sp2 = CLFont::snapPoint(p2);

  lines_.push_back(new CLFontLine(sp1, sp2));

  return int(lines_.size() - 1);
}

const CLFontLine &
CLFontDef::
getLine(int i) const
{
  return *lines_[uint(i)];
}

void
CLFontDef::
setLine(int i, const CLFontLine &line)
{
  *lines_[uint(i)] = line;
}

void
CLFontDef::
deleteLine(int i)
{
  delete lines_[uint(i)];

  for (int j = i + 1; j < int(lines_.size()); ++j)
    lines_[uint(j - 1)] = lines_[uint(j)];

  lines_.pop_back();
}

//----

const CLFontDef &
CLFont::
getFontDef(char c)
{
  if (c >= ' ' && c <= '~')
    return lfont_defs[c - ' '];
  else
    return lfont_defs[0];
}

void
CLFont::
setFontDef(char c, const CLFontDef &fontDef)
{
  if (c >= ' ' && c <= '~')
    lfont_defs[c - ' '] = fontDef;
}

CPoint2D
CLFont::
snapPoint(const CPoint2D &p)
{
  return CPoint2D(std::round(p.x*16.0)/16.0, std::round(p.y*16.0)/16.0);
}
