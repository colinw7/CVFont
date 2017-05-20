#ifndef CQLFontEd_H
#define CQLFontEd_H

#include <CLFont.h>
#include <C3Bezier2D.h>
#include <CQUtil.h>
#include <CDisplayRange2D.h>

#include <CQApp.h>
#include <CQMenu.h>
#include <CQMainWindow.h>

#include <QWidget>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>

class Application;

class Canvas : public QWidget {
  Q_OBJECT

 public:
  Canvas(Application *app, const std::string &name);

  const CDisplayRange2D &range() const { return range_; }

 private:
  void paintEvent(QPaintEvent *);

  void resizeEvent(QResizeEvent *);

  void keyPressEvent(QKeyEvent *);

  void mousePressEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);

 private:
  Application     *app_;
  CDisplayRange2D  range_;
};

//---

class Preview : public QWidget {
  Q_OBJECT

 public:
  Preview(Application *app, const std::string &name);

 private:
  void paintEvent(QPaintEvent *);

  void resizeEvent(QResizeEvent *);

 private:
  Application     *app_;
  CDisplayRange2D  range_;
};

//---

class PointData {
 public:
  PointData(int ind=-1, int subInd=-1) :
   ind_(ind), subInd_(subInd) {
  }

  bool isSet() const { return ind_ >= 0; }

  void reset() { ind_ = -1; subInd_ = -1; }

  int ind() const { return ind_; }
  void setInd(int i) { ind_ = i; }

  int subInd() const { return subInd_; }
  void setSubInd(int i) { subInd_ = i; }

  bool operator==(const PointData &data) const {
    return (ind_ == data.ind_ && subInd_ == data.subInd_);
  }

 private:
  int ind_    { -1 };
  int subInd_ { -1 };
};

//---

class Application : public CQMainWindow {
  Q_OBJECT

 public:
  enum class State {
    NONE,
    MOVE,
    ADD,
    DELETE
  };

  enum class MoveType {
    NONE,
    TO,
    BY
  };

 public:
  Application(const std::string &name);

  double lineWidth() const { return lineWidth_; }
  void setLineWidth(double r) { lineWidth_ = r; }

  void createMenus();

  QWidget *createCentralWidget();

  void createStatusBar();

  void draw(QPainter *painter);

  void drawText(QPainter *painter, const CPoint2D &p, const std::string &str);

  void drawChar(QPainter *painter, const CPoint2D &p, const CLFontDef &fontDef);

  void drawSelected(QPainter *painter);

  void drawPreviewPoint(QPainter *painter);

  void drawPointData(QPainter *painter, const PointData &pointData, const QColor &c);

  void drawCrossPoint(QPainter *painter, const QPointF &p, const QColor &c);

  void setKey(char c);

  void mousePress  (const CPoint2D &p);
  void mouseMove   (const CPoint2D &p);
  void mouseRelease(const CPoint2D &p);

  void selectAt(const CPoint2D &p);

  void escape();

  void nextChar();
  void prevChar();

  void moveTo(const CPoint2D &p);
  void moveBy(const CPoint2D &p);

  void move(const MoveType &moveType, const CPoint2D &p);

  void deleteSelected();

  PointData nearestPoint(const CPoint2D &p) const;

  CPoint2D pointDataPos(const PointData &pointData) const;

  void updateFontDef();

  QSize sizeHint() const { return QSize(800, 800); }

 private slots:
  void print();
  void printAll();

  void preview();

  void increaseLineWidth();
  void decreaseLineWidth();

  void moveState();
  void addState();
  void deleteState();

  void updateAll();

  void updateState();

 private:
  void printChar(CFile &file, char c, const CLFontDef &fontDef);

 private:
  struct MouseState {
    bool     pressed { false };
    bool     moving { false };
    bool     escape { false };
    CPoint2D pressPoint;
    CPoint2D movePoint;
    CPoint2D releasePoint;
  };

  class FontData {
   public:
    FontData(char c) {
      setChar(c);
    }

    char c() const { return c_; }

    const CLFontDef &fontDef() const { return fontDef_; }
    CLFontDef &fontDef() { return fontDef_; }

    int charNum() const { return int(c_ - ' '); }

    void setChar(char c) {
      c_       = c;
      fontDef_ = CLFont::getFontDef(c_);
    }

   public:
    char      c_ { '\0' };
    CLFontDef fontDef_;
  };

  State      state_ { State::MOVE };
  Canvas*    canvas_ { nullptr };
  QLabel*    stateLabel_ { nullptr };
  QLabel*    posLabel_ { nullptr };
  Preview*   preview_ { nullptr };
  FontData   fontData_;
  PointData  selectedPoint_;
  PointData  previewPoint_;
  CPoint2D   selectPoint_;
  MouseState mouseState_;
  bool       moving_ { false };
  int        lineWidth_ { 1 };
};

#endif
