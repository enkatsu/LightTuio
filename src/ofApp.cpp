#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
#ifdef MOVIE_DEBUG
    testPlayer.load("movies/test.mov");
    testPlayer.setLoopState(ofLoopType::OF_LOOP_NORMAL);
    testPlayer.setSpeed(0.5);
    testPlayer.play();
    cameraWidth = testPlayer.getWidth();
    cameraHeight = testPlayer.getHeight();
#else
    camera.setDeviceID(0);
    camera.initGrabber(1280, 720);
    cameraWidth = camera.getWidth();
    cameraHeight = camera.getHeight();
#endif
    
    ofRectangle screenRect = ofRectangle(0, 0, ofGetScreenWidth(), ofGetScreenHeight());
    ofRectangle cameraRect = ofRectangle(0, 0, cameraWidth, cameraHeight);
    ofRectangle windowRect = ofRectangle(0, 0, cameraRect.width * 2, cameraRect.height);
    windowRect.scaleTo(screenRect);
    windowRect.x = 0;
    windowRect.y = 0;
    ofSetWindowShape(windowRect.width, windowRect.height);
    
    grayImg.allocate(cameraWidth, cameraHeight);
    colorBg.allocate(cameraWidth, cameraHeight);
    grayBg.allocate(cameraWidth, cameraHeight);
    
    cornerPoints.push_back(ofPoint(windowRect.x, windowRect.y));
    cornerPoints.push_back(ofPoint(windowRect.x + windowRect.width / 2, windowRect.y));
    cornerPoints.push_back(ofPoint(windowRect.x + windowRect.width / 2, windowRect.y + windowRect.height));
    cornerPoints.push_back(ofPoint(windowRect.x, windowRect.y + windowRect.height));
    
    gui.setup();
    gui.add(usage.setup("usage", "d: draw"));
    gui.add(threshold.setup("threshold", 50, 0, 255));
    gui.add(minArea.setup("min area", cameraWidth * cameraHeight / 10, 0, cameraWidth * cameraHeight));
    gui.add(maxArea.setup("max area", cameraWidth * cameraHeight / 2, 0, cameraWidth * cameraHeight));
    gui.add(blur.setup("blur", 15, 0, 20));
    gui.add(invert.setup("invert", false));
    gui.add(mirror.setup("mirror", false));
}

//--------------------------------------------------------------
void ofApp::update(){
#ifdef MOVIE_DEBUG
    testPlayer.update();
    if (testPlayer.isFrameNew()) {
        ofPixels pixels = testPlayer.getPixels();
#else
    camera.update();
    if (camera.isFrameNew()) {
        ofPixels pixels = camera.getPixels();
#endif
        pixels.mirror(false, mirror);
        if (bLearnBackground) {
            colorBg.setFromPixels(pixels);
            grayBg.setFromColorImage(colorBg);
        }
        colorImg.setFromPixels(pixels);
        warpPerspectivedImg.setFromPixels(pixels);
        vector<ofPoint> wpPoints;
        float originMaxWidth = ofGetWidth() / 2;
        float originMaxHeight = ofGetHeight();
        wpPoints.push_back(ofPoint(ofMap(cornerPoints.at(0).x, 0, originMaxWidth, 0, cameraWidth),
                                   ofMap(cornerPoints.at(0).y, 0, originMaxHeight, 0, cameraHeight)));
        wpPoints.push_back(ofPoint(ofMap(cornerPoints.at(1).x, 0, originMaxWidth, 0, cameraWidth),
                                   ofMap(cornerPoints.at(1).y, 0, originMaxHeight, 0, cameraHeight)));
        wpPoints.push_back(ofPoint(ofMap(cornerPoints.at(2).x, 0, originMaxWidth, 0, cameraWidth),
                                   ofMap(cornerPoints.at(2).y, 0, originMaxHeight, 0, cameraHeight)));
        wpPoints.push_back(ofPoint(ofMap(cornerPoints.at(3).x, 0, originMaxWidth, 0, cameraWidth),
                                   ofMap(cornerPoints.at(3).y, 0, originMaxHeight, 0, cameraHeight)));
        warpPerspectivedImg.warpPerspective(wpPoints.at(0), wpPoints.at(1), wpPoints.at(2), wpPoints.at(3));
        
        grayImg.setFromColorImage(warpPerspectivedImg);
        if (invert) {
            grayImg.invert();
        }
        grayImg.blur(blur);
        grayDiff.absDiff(grayBg, grayImg);
        grayDiff.threshold(threshold);
        contourFinder.findContours(grayDiff, minArea, maxArea, 4, false, true);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
#ifdef MOVIE_DEBUG
    testPlayer.draw(0, 0, ofGetWidth() / 2, ofGetHeight());
    grayDiff.draw(ofGetWidth() / 2, 0, ofGetWidth() / 2, ofGetHeight());
#else
    camera.draw(0, 0, ofGetWidth() / 2, ofGetHeight());
    grayDiff.draw(ofGetWidth() / 2, 0, ofGetWidth() / 2, ofGetHeight());
#endif
    
    drawContourInfo();
    
    // *** corner points ***
    ofPushStyle();
    for (int i = 0; i < cornerPoints.size(); i++) {
        if (i == cornerPointDraggedIndex) {
            ofSetColor(ofColor::red);
        } else {
            ofSetColor(ofColor::white);
        }
        ofVec2f p = cornerPoints.at(i);
        ofDrawCircle(p.x, p.y, cornerPointRadius);
    }
    ofPopStyle();
    
    gui.draw();
}
    
void ofApp::drawContourInfo() {
    ofPushStyle();
    for(int i = 0; i < contourFinder.nBlobs; i++) {
        ofxCvBlob blob = contourFinder.blobs.at(i);
        ofColor c;
        c.setHsb(i * 64, 255, 255);
        ofNoFill();
        ofSetColor(c);
        
        ofRectangle r = blob.boundingRect;
        r.x = ofMap(r.x, 0, grayDiff.width, ofGetWidth() / 2, ofGetWidth());
        r.y = ofMap(r.y, 0, grayDiff.height, 0, ofGetHeight());
        r.width = ofMap(r.width, 0, grayDiff.width, 0, ofGetWidth() / 2);
        r.height = ofMap(r.height, 0, grayDiff.height, 0, ofGetHeight());
        ofDrawRectangle(r);
        
        ofDefaultVec3 centroid = blob.centroid;
        centroid.x = ofMap(centroid.x, 0, grayDiff.width, ofGetWidth() / 2, ofGetWidth());
        centroid.y = ofMap(centroid.y, 0, grayDiff.height, 0, ofGetHeight());
        ofDrawCircle(centroid.x, centroid.y, 5);
        
        ofBeginShape();
        ofSetColor(c, 100);
        ofFill();
        for (int j = 0; j < blob.nPts; j++) {
            ofDefaultVec3 vec = blob.pts.at(j);
            vec.x = ofMap(vec.x, 0, grayDiff.width, 0, ofGetWidth() / 2, ofGetWidth());
            vec.y = ofMap(vec.y, 0, grayDiff.height, 0, ofGetHeight());
            ofVertex(vec.x, vec.y);
        }
        ofEndShape();
    }
    ofPopStyle();
}

void ofApp::drawImages() {
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    if (cornerPointDraggedIndex >= 0) {
        ofPoint &p = cornerPoints.at(cornerPointDraggedIndex);
        p.x = x;
        p.y = y;
        if (p.x < 0) {
            p.x = 0;
        }
        if (p.x > ofGetWidth() / 2) {
            p.x = ofGetWidth() / 2;
        }
        if (p.y < 0) {
            p.y = 0;
        }
        if (p.y > ofGetHeight()) {
            p.y = ofGetHeight();
        }
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    for (int i = 0; i < cornerPoints.size(); i++) {
        ofVec2f p = cornerPoints.at(i);
        float d = p.distance(ofVec2f(x, y));
        if (d < cornerPointRadius) {
            cornerPointDraggedIndex = i;
            break;
        }
    }
    cout << cornerPointDraggedIndex << endl;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    if (cornerPointDraggedIndex != -1) {
        cornerPointDraggedIndex = -1;
    }
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}

void ofApp::setupGui() {
    gui.setup();
    gui.add(usage.setup("usage", "d: draw"));
    gui.add(threshold.setup("threshold", 50, 0, 255));
    gui.add(minArea.setup("min area", cameraWidth * cameraHeight / 10, 0, cameraWidth * cameraHeight));
    gui.add(maxArea.setup("max area", cameraWidth * cameraHeight / 2, 0, cameraWidth * cameraHeight));
    gui.add(blur.setup("blur", 15, 0, 20));
    gui.add(invert.setup("invert", false));
    gui.add(mirror.setup("mirror", false));
}
