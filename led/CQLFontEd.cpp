#include <CQLFontEd.h>
#include <CQUtilGeom.h>
#include <CFile.h>

#include <QStatusBar>
#include <QLabel>

int
main(int argc, char **argv)
{
  CQApp qapp(argc, argv);

  Application *app = new Application("lfont");

  app->init();

  app->show();

  return qapp.exec();
}

Application::
Application(const std::string &name) :
 CQMainWindow(), fontData_('Q')
{
  setObjectName(name.c_str());
}

QWidget *
Application::
createCentralWidget()
{
  canvas_ = new Canvas(this, "canvas");

  return canvas_;
}

void
Application::
createStatusBar()
{
  stateLabel_ = new QLabel;
  posLabel_   = new QLabel;

  statusBar()->addPermanentWidget(stateLabel_, 1);
  statusBar()->addPermanentWidget(posLabel_);

  stateLabel_->setText(" ");
  posLabel_  ->setText(" ");

  updateState();
}

void
Application::
createMenus()
{
  CQMenu *fileMenu = new CQMenu(this, "&File");

  CQMenuItem *quitItem = new CQMenuItem(fileMenu, "&Quit");

  quitItem->setShortcut("Ctrl+Q");
  quitItem->setStatusTip("Quit the application");

  quitItem->connect(this, SLOT(close()));

  CQMenuItem *printItem = new CQMenuItem(fileMenu, "&Print");

  printItem->setShortcut("Ctrl+P");
  printItem->setStatusTip("Print the current character");
  printItem->connect(this, SLOT(print()));

  CQMenuItem *printAllItem = new CQMenuItem(fileMenu, "&Print All");

  printAllItem->connect(this, SLOT(printAll()));

  CQMenuItem *helpItem = new CQMenuItem(fileMenu, "&Help");

  helpItem->setShortcut("Ctrl+H");
  helpItem->setStatusTip("Help");

  //helpItem->connect(this, SLOT(help()));

  //---

  CQMenu *viewMenu = new CQMenu(this, "&View");

  CQMenuItem *previewItem = new CQMenuItem(viewMenu, "&Preview");

  previewItem->setStatusTip("Preview font with example text");
  previewItem->connect(this, SLOT(preview()));

  CQMenuItem *increaseLineWidthItem = new CQMenuItem(viewMenu, "&Increase Line Width");

  increaseLineWidthItem->connect(this, SLOT(increaseLineWidth()));

  CQMenuItem *decreaseLineWidthItem = new CQMenuItem(viewMenu, "&Decrease Line Width");

  decreaseLineWidthItem->connect(this, SLOT(decreaseLineWidth()));

  //---

  CQMenu *editMenu = new CQMenu(this, "&Edit");

  editMenu->startGroup();

  CQMenuItem *moveItem   = new CQMenuItem(editMenu, "&Move"  , CQMenuItem::CHECKED);
  CQMenuItem *addItem    = new CQMenuItem(editMenu, "&Add"   , CQMenuItem::CHECKABLE);
  CQMenuItem *deleteItem = new CQMenuItem(editMenu, "&Delete", CQMenuItem::CHECKABLE);

  editMenu->endGroup();

  moveItem  ->connect(this, SLOT(moveState()));
  addItem   ->connect(this, SLOT(addState()));
  deleteItem->connect(this, SLOT(deleteState()));
}

void
Application::
print()
{
  CFile file("/tmp/CLFontDef.h");

  if (! file.open(CFile::Mode::WRITE))
    return;

  printChar(file, fontData_.c(), fontData_.fontDef());

  file.close();
}

void
Application::
printAll()
{
  CFile file("/tmp/CLFontDef.h");

  if (! file.open(CFile::Mode::WRITE))
    return;

  for (char c = ' '; c <= '~'; ++c) {
    const CLFontDef &fontDef = CLFont::getFontDef(c);

    printChar(file, c, fontDef);
  }

  file.close();
}

void
Application::
printChar(CFile &file, char c, const CLFontDef &fontDef)
{
  int charNum = c - ' ';

  //---

  bool lineFound = false;

  for (const auto &line : fontDef.lines()) {
    if (! lineFound) {
      file.writef("static CLFontLine lines%02d[] = { /* %c */\n", charNum, c);

      lineFound = true;
    }

    file.writef(" {{% 6.4f,% 6.4f},{% 6.4f,% 6.4f}},\n",
                line->start().x, line->start().y, line->end().x, line->end().y);
  }

  if (lineFound)
    file.write("};\n");

  //---

  file.writef("/* %c */\n", c);
  file.writef("static std::string lines%02d = \"\\\n", charNum);

  for (const auto &line : fontDef.lines()) {
    file.writef(" L %6.4f %6.4f %6.4f %6.4f \\\n",
                line->start().x, line->start().y, line->end().x, line->end().y);
  }

  file.write("\";\n");

  //---

  file.close();
}

void
Application::
draw(QPainter *painter)
{
  QPen pen;

  pen.setWidth(1);
  pen.setCosmetic(true);
  pen.setColor(Qt::green);

  painter->setPen(pen);

  double x1 = 0.0;
  double x2 = CLFont::charWidth;
  double y1 = -CLFont::charDescent;
  double y2 = 0.0;
  double y3 = y2 + CLFont::charAscent;

  painter->drawLine(QPointF(x1, y1), QPointF(x2, y1));
  painter->drawLine(QPointF(x2, y1), QPointF(x2, y3));
  painter->drawLine(QPointF(x2, y3), QPointF(x1, y3));
  painter->drawLine(QPointF(x1, y3), QPointF(x1, y1));

  pen.setColor(QColor(40,40,40));

  painter->setPen(pen);

  double dx = 0.0625;
  double dy = 0.0625;

  for (double x = x1 + dx; x <= x2 - dx; x += dx) {
    painter->drawLine(QPointF(x, y1), QPointF(x, y3));
  }

  for (double y = y1 + dy; y <= y3 - dy; y += dy) {
    painter->drawLine(QPointF(x1, y), QPointF(x2, y));
  }

  pen.setColor(Qt::blue);

  painter->setPen(pen);

  painter->drawLine(QPointF(x1, y2), QPointF(x2, y2));

  drawChar(painter, CPoint2D(0, 0), fontData_.fontDef());

  drawPreviewPoint(painter);

  drawSelected(painter);

  //---

  if (state_ == State::ADD && moving_ && ! mouseState_.escape) {
    pen.setColor(Qt::yellow);

    painter->setPen(pen);

    painter->drawLine(CQUtil::toQPoint(mouseState_.pressPoint),
                      CQUtil::toQPoint(mouseState_.movePoint));
  }
}

void
Application::
drawText(QPainter *painter, const CPoint2D &p, const std::string &str)
{
  CPoint2D p1 = p;

  for (std::size_t i = 0; i < str.size(); ++i) {
    const CLFontDef &fontDef = CLFont::getFontDef(str[i]);

    drawChar(painter, p1, fontDef);

    p1 += CPoint2D(1, 0);
  }
}

void
Application::
drawChar(QPainter *painter, const CPoint2D &p, const CLFontDef &fontDef)
{
  QPen pen;

  pen.setWidth(lineWidth());
  pen.setCosmetic(true);
  pen.setColor(Qt::white);

  painter->setPen(pen);

  int lastXInd  = -1;
  int lastYInd  = -1;
  int firstXInd = -1;
  int firstYInd = -1;

  QPainterPath path;

  for (const auto &line : fontDef.lines()) {
    QPointF p1(p.x + line->start().x, p.y + line->start().y);
    QPointF p2(p.x + line->end  ().x, p.y + line->end  ().y);

    int xind1 = line->startXInd();
    int yind1 = line->startYInd();
    int xind2 = line->endXInd();
    int yind2 = line->endYInd();

    if (xind1 != lastXInd || yind1 != lastYInd) {
      if (xind2 == firstXInd && yind2 == firstYInd)
        path.closeSubpath();

      firstXInd = xind1;
      firstYInd = yind1;

      path.moveTo(p1);
    }

    path.lineTo(p2);

    lastXInd = xind2;
    lastYInd = yind2;
  }

  painter->strokePath(path, pen);
}

void
Application::
drawSelected(QPainter *painter)
{
  if (! selectedPoint_.isSet())
    return;

  const CLFontLine *line = fontData_.fontDef().lines()[selectedPoint_.ind()];

  QPen pen;

  pen.setWidth(1);
  pen.setCosmetic(true);
  pen.setColor(Qt::gray);
  pen.setStyle(Qt::DotLine);

  painter->setPen(pen);

  drawCrossPoint(painter, CQUtil::toQPoint(line->start()), Qt::gray);
  drawCrossPoint(painter, CQUtil::toQPoint(line->end  ()), Qt::gray);

  drawPointData(painter, selectedPoint_, Qt::red);
}

void
Application::
drawPreviewPoint(QPainter *painter)
{
  drawPointData(painter, previewPoint_, Qt::yellow);
}

void
Application::
drawPointData(QPainter *painter, const PointData &pointData, const QColor &c)
{
  if (pointData.isSet()) {
    const CLFontLine *line = fontData_.fontDef().lines()[pointData.ind()];

    if      (pointData.subInd() == 0)
      drawCrossPoint(painter, CQUtil::toQPoint(line->start()), c);
    else if (pointData.subInd() == 1)
      drawCrossPoint(painter, CQUtil::toQPoint(line->end  ()), c);
  }
}

void
Application::
drawCrossPoint(QPainter *painter, const QPointF &p, const QColor &c)
{
  QPen pen;

  pen.setWidth(1);
  pen.setCosmetic(true);
  pen.setColor(c);

  painter->setPen(pen);

  double s = 0.02;

  painter->drawLine(p - QPointF( s, s), p + QPointF( s, s));
  painter->drawLine(p - QPointF(-s, s), p + QPointF(-s, s));
}

void
Application::
setKey(char c)
{
  fontData_.setChar(c);

  selectedPoint_.reset();
  previewPoint_ .reset();

  updateAll();
}

void
Application::
mousePress(const CPoint2D &p)
{
  mouseState_.pressed    = true;
  mouseState_.pressPoint = CLFont::snapPoint(p);
  mouseState_.moving     = false;
  mouseState_.escape     = false;

  if      (state_ == State::MOVE) {
    selectAt(mouseState_.pressPoint);

    selectPoint_ = pointDataPos(selectedPoint_);
  }
  else if (state_ == State::DELETE) {
    selectAt(mouseState_.pressPoint);

    deleteSelected();
  }
}

void
Application::
mouseMove(const CPoint2D &p)
{
  mouseState_.movePoint = CLFont::snapPoint(p);

  posLabel_->setText(QString("%1,%2").arg(mouseState_.movePoint.x).arg(mouseState_.movePoint.y));

  if (mouseState_.pressed) {
    mouseState_.moving = true;

    if (mouseState_.escape)
      return;

    if      (state_ == State::MOVE)
      moveTo(p);
    else if (state_ == State::ADD)
      updateAll();
  }
  else {
    previewPoint_ = nearestPoint(mouseState_.movePoint);

    updateAll();
  }
}

void
Application::
mouseRelease(const CPoint2D &p)
{
  mouseState_.pressed      = false;
  mouseState_.releasePoint = CLFont::snapPoint(p);
  mouseState_.moving       = false;
  mouseState_.escape       = false;

  if (state_ == State::ADD) {
    if (mouseState_.escape)
      return;

    int ind = fontData_.fontDef().addLine(mouseState_.pressPoint, mouseState_.releasePoint);

    selectedPoint_ = PointData(ind, 0);

    updateFontDef();

    updateAll();
  }
}

void
Application::
selectAt(const CPoint2D &p)
{
  selectedPoint_ = nearestPoint(p);

  std::cerr << pointDataPos(selectedPoint_) << std::endl;

  updateAll();
}

void
Application::
escape()
{
  mouseState_.escape = true;

  if (mouseState_.moving && state_ == State::MOVE)
    moveTo(selectPoint_);
}

void
Application::
nextChar()
{
  int charNum = fontData_.charNum();

  if (charNum < 94) {
    ++charNum;

    setKey(charNum + ' ');

    updateAll();
  }
}

void
Application::
prevChar()
{
  int charNum = fontData_.charNum();

  if (charNum > 0) {
    --charNum;

    setKey(charNum + ' ');

    updateAll();
  }
}

void
Application::
moveTo(const CPoint2D &p)
{
  move(MoveType::TO, p);
}

void
Application::
moveBy(const CPoint2D &p)
{
  move(MoveType::BY, p);
}

void
Application::
move(const MoveType &moveType, const CPoint2D &p)
{
  CPoint2D p1 = CLFont::snapPoint(p);

  if (selectedPoint_.isSet()) {
    CLFontLine *line = fontData_.fontDef_.lines()[selectedPoint_.ind()];

    if      (selectedPoint_.subInd() == 0) {
      if      (moveType == MoveType::TO)
        line->setStart(p1);
      else if (moveType == MoveType::BY)
        line->setStart(line->start() + p1);
    }
    else if (selectedPoint_.subInd() == 1) {
      if      (moveType == MoveType::TO)
        line->setEnd(p1);
      else if (moveType == MoveType::BY)
        line->setEnd(line->end() + p1);
    }

    updateFontDef();

    updateAll();
  }
}

void
Application::
deleteSelected()
{
  if (selectedPoint_.isSet()) {
    fontData_.fontDef_.deleteLine(selectedPoint_.ind());

    selectedPoint_.reset();
    previewPoint_ .reset();

    updateFontDef();
  }
}

PointData
Application::
nearestPoint(const CPoint2D &p) const
{
  class MinDist {
   public:
    MinDist(const CPoint2D &p) :
     p_(p) {
    }

    double minDist() const { return minD_; }

    const PointData &minPoint() const { return minPoint_; }

    bool update(const PointData &pointData, const CPoint2D &p) {
      double d = std::hypot(p_.x - p.x, p_.y - p.y);

      if (! minPoint_.isSet() || d < minD_) {
        minD_     = d;
        minPoint_ = pointData;
        minP_     = p;
      }

      return (minD_ < 1E-5);
    }

   private:
    CPoint2D  p_;
    double    minD_ { 1E50 };
    PointData minPoint_;
    CPoint2D  minP_;
  };

  //---

  MinDist minDist(p);

  int i = 0;

  for (const auto &line : fontData_.fontDef_.lines()) {
    if (minDist.update(PointData(i, 0), line->start())) break;
    if (minDist.update(PointData(i, 1), line->end  ())) break;

    ++i;
  }

  if (minDist.minDist() > 1E-5)
    return PointData();

  return minDist.minPoint();
}

CPoint2D
Application::
pointDataPos(const PointData &pointData) const
{
  if (selectedPoint_.isSet()) {
    const CLFontLine *line = fontData_.fontDef_.lines()[selectedPoint_.ind()];

    if      (pointData.subInd() == 0) return line->start();
    else if (pointData.subInd() == 1) return line->end  ();
  }

  return CPoint2D(0, 0);
}

void
Application::
updateFontDef()
{
  CLFont::setFontDef(fontData_.c(), fontData_.fontDef());

  updateAll();
}

void
Application::
preview()
{
  if (! preview_)
    preview_ = new Preview(this, "Preview");

  preview_->show();
}

void
Application::
increaseLineWidth()
{
  lineWidth_ += 1;

  updateAll();
}

void
Application::
decreaseLineWidth()
{
  if (lineWidth_ > 1) {
    lineWidth_ -= 1;

    updateAll();
  }
}

void
Application::
moveState()
{
  state_ = State::MOVE;

  updateState();
}

void
Application::
addState()
{
  state_ = State::ADD;

  updateState();
}

void
Application::
deleteState()
{
  state_ = State::DELETE;

  updateState();
}

void
Application::
updateAll()
{
  update();

  if (preview_)
    preview_->update();

  updateState();
}

void
Application::
updateState()
{
  QString str;

  if      (state_ == State::MOVE)
    str += "Move";
  else if (state_ == State::ADD)
    str += "Add";
  else if (state_ == State::DELETE)
    str += "Delete";

  str += QString(" : Char %1 (#%2)").arg(fontData_.c()).arg(fontData_.charNum());

  stateLabel_->setText(str);
}

//---------

Canvas::
Canvas(Application *app, const std::string &name) :
 QWidget(app), app_(app)
{
  setObjectName(name.c_str());

  double margin = CLFont::charWidth/2.0;

  range_.setWindowRange(-margin, -margin - CLFont::charDescent,
                        CLFont::charWidth + margin, CLFont::charAscent + margin);

  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);
}

void
Canvas::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.setRenderHints(QPainter::Antialiasing);

  painter.fillRect(rect(), QBrush(QColor(0,0,0)));

  painter.setWorldTransform(CQUtil::toQTransform(range_.getMatrix()));

  app_->draw(&painter);
}

void
Canvas::
resizeEvent(QResizeEvent *)
{
  range_.setPixelRange(0, 0, width() - 1, height() - 1);
}

void
Canvas::
keyPressEvent(QKeyEvent *e)
{
  if      (e->key() == Qt::Key_Left)
    app_->moveBy(CPoint2D(-CLFont::delta, 0));
  else if (e->key() == Qt::Key_Right)
    app_->moveBy(CPoint2D( CLFont::delta, 0));
  else if (e->key() == Qt::Key_Down)
    app_->moveBy(CPoint2D(0, -CLFont::delta));
  else if (e->key() == Qt::Key_Up)
    app_->moveBy(CPoint2D(0,  CLFont::delta));
  else if (e->key() == Qt::Key_Escape)
    app_->escape();
  else if (e->key() == Qt::Key_PageDown)
    app_->prevChar();
  else if (e->key() == Qt::Key_PageUp)
    app_->nextChar();
  else {
    QString text = e->text();

    if (text.length())
      app_->setKey(text[0].toLatin1());
  }

  update();
}

void
Canvas::
mousePressEvent(QMouseEvent *e)
{
  CPoint2D p;

  range_.pixelToWindow(CPoint2D(e->x(), e->y()), p);

  app_->mousePress(p);
}

void
Canvas::
mouseMoveEvent(QMouseEvent *e)
{
  CPoint2D p;

  range_.pixelToWindow(CPoint2D(e->x(), e->y()), p);

  app_->mouseMove(p);
}

void
Canvas::
mouseReleaseEvent(QMouseEvent *e)
{
  CPoint2D p;

  range_.pixelToWindow(CPoint2D(e->x(), e->y()), p);

  app_->mouseRelease(p);
}

//---------

Preview::
Preview(Application *app, const std::string &name) :
 QWidget(), app_(app)
{
  setObjectName(name.c_str());

  range_.setWindowRange(0, 0, 30, 20);

  setFocusPolicy(Qt::StrongFocus);
}

void
Preview::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.setRenderHints(QPainter::Antialiasing);

  painter.fillRect(rect(), QBrush(QColor(0,0,0)));

  painter.setWorldTransform(CQUtil::toQTransform(range_.getMatrix()));

  double y  = 20;
  double dy = CLFont::charHeight;

  y -= dy; app_->drawText(&painter, CPoint2D(0, y), "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  y -= dy; app_->drawText(&painter, CPoint2D(0, y), "abcdefghijklmnopqrstuvwxyz");
  y -= dy; app_->drawText(&painter, CPoint2D(0, y), "1234567890!\"£$%^&*()-_=+\\|");
  y -= dy; app_->drawText(&painter, CPoint2D(0, y), "'~[]{}:;@#<>?,./");
}

void
Preview::
resizeEvent(QResizeEvent *)
{
  range_.setPixelRange(0, 0, width() - 1, height() - 1);
}
