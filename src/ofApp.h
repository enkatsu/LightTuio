#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"
#include "ofxTuioServer.h"
#include "ofxXmlSettings.h"
#define MOVIE_DEBUG
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void setupGui();
    void drawImages();
    void drawContourInfo();
    
#ifdef MOVIE_DEBUG
    ofVideoPlayer testPlayer;
#else
    ofVideoGrabber camera;
#endif
    bool isStop = true;
    
    int cameraWidth, cameraHeight;
    ofxCvColorImage colorImg, warpPerspectivedImg, colorBg;
    ofxCvGrayscaleImage grayImg, grayDiff, grayBg;
    ofxCvContourFinder contourFinder;
    bool bLearnBackground = false;
    vector<ofPoint> cornerPoints;
    float cornerPointRadius = 20;
    int cornerPointDraggedIndex = -1;
    
    ofxPanel gui;
    ofxFloatSlider threshold;
    ofxFloatSlider minArea, maxArea;
    ofxIntSlider blur;
    ofxToggle invert;
    ofxToggle mirror;
    ofxToggle normalize;
    ofxToggle send;
    ofxLabel usage;
    
    ofxTuioServer myTuioServer;
    TuioCursor * cursor;
};
