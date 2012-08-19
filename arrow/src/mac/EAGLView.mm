//
//  EAGLView.m
//  flight
//
//  Created by mac on 09-8-8.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//



#import "EAGLView.h"
#include "lwFramework/lwApp.h"
#include "lwFramework/lwTask.h"
#include "lwFramework/lwUIWidget.h"

#define USE_DEPTH_BUFFER 1

// A class extension to declare private methods
@interface EAGLView ()

@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, assign) NSTimer *animationTimer;

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

@end


@implementation EAGLView

@synthesize context;
@synthesize animationTimer;
@synthesize animationInterval;


// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}


//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithCoder:(NSCoder*)coder {
    
    if ((self = [super initWithCoder:coder])) {
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!context || ![EAGLContext setCurrentContext:context]) {
            [self release];
            return nil;
        }
        
        animationInterval = 1.0 / GAME_FPS;
		cfTime = CFAbsoluteTimeGetCurrent();
		
		self.multipleTouchEnabled = TRUE;
    }
    return self;
}


- (void)drawView {    
    [EAGLContext setCurrentContext:context];
	
	CFAbsoluteTime t =  CFAbsoluteTimeGetCurrent();
	CFTimeInterval dt = t - cfTime;
	cfTime = t;
	dt *= 1000.f;
	dt = min(dt, 100.0);
	lw::App::main(dt);
	
//	static bool needDraw = false;
//	needDraw = !needDraw;
//	if ( !needDraw ){
//		return;
//	}
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glViewport(0, 0, backingWidth, backingHeight);
    
    //glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	lw::App::draw(dt);
    
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];
}


- (void)layoutSubviews {
    [EAGLContext setCurrentContext:context];
    [self destroyFramebuffer];
    [self createFramebuffer];
    [self drawView];
}


- (BOOL)createFramebuffer {
    
    glGenFramebuffersOES(1, &viewFramebuffer);
    glGenRenderbuffersOES(1, &viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
    
    if (USE_DEPTH_BUFFER) {
        glGenRenderbuffersOES(1, &depthRenderbuffer);
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
        glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
    }
    
    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
    
    return YES;
}


- (void)destroyFramebuffer {
    glDeleteFramebuffersOES(1, &viewFramebuffer);
    viewFramebuffer = 0;
    glDeleteRenderbuffersOES(1, &viewRenderbuffer);
    viewRenderbuffer = 0;
    
    if(depthRenderbuffer) {
        glDeleteRenderbuffersOES(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
}


- (void)startAnimation {
    self.animationTimer = [NSTimer scheduledTimerWithTimeInterval:animationInterval target:self selector:@selector(drawView) userInfo:nil repeats:YES];
}


- (void)stopAnimation {
    self.animationTimer = nil;
}


- (void)setAnimationTimer:(NSTimer *)newTimer {
    [animationTimer invalidate];
    animationTimer = newTimer;
}


- (void)setAnimationInterval:(NSTimeInterval)interval {
    
    animationInterval = interval;
    if (animationTimer) {
        [self stopAnimation];
        [self startAnimation];
    }
}


- (void)dealloc {
    
    [self stopAnimation];
    
    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    [context release];  
    [super dealloc];
}

void transXY(lw::TouchEvent& evt, CGPoint& pt, CGPoint& prevPt){
	lw::App::Orientation orientation = lw::App::getConfig().orientation;
	switch (orientation) {
		case lw::App::ORIENTATION_UP:
			evt.x = pt.x;
			evt.y = pt.y;
			evt.prevX = prevPt.x;
			evt.prevY = prevPt.y;
			break;
		case lw::App::ORIENTATION_RIGHT:
			evt.x = pt.y;
			evt.y = 320-pt.x;
			evt.prevX = prevPt.y;
			evt.prevY = 320 - prevPt.x;
			break;
		case lw::App::ORIENTATION_LEFT:
			evt.x = 480-pt.y;
			evt.y = pt.x;
			evt.prevX = 480-prevPt.y;
			evt.prevY = prevPt.x;
			break;
		default:
			break;
	}
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event{
	std::vector<lw::TouchEvent> events;
	for ( UITouch* touch in touches ){
		lw::TouchEvent evt;
		evt.type = lw::TouchEvent::TOUCH;
		CGPoint pt = [touch locationInView:self];
		CGPoint prevPt = [touch previousLocationInView:self];
		transXY(evt, pt, prevPt);
		//lwinfo(evt.x << ":" << evt.y);
		events.push_back(evt);
	}
	lw::g_gestrueMgr.onTouchEvent(events);
	
	if ( !lw::uiEvent(events) ){
		lw::TaskMgr::onTouchEvent(events);
	}
	lw::g_gestrueMgr.main();
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event{
	std::vector<lw::TouchEvent> events;
	//lwinfo([touches count]);
	for ( UITouch* touch in touches ){
		lw::TouchEvent evt;
		evt.type = lw::TouchEvent::MOVE;
		CGPoint pt = [touch locationInView:self];
		CGPoint prevPt = [touch previousLocationInView:self];
		transXY(evt, pt, prevPt);
		//lwinfo(evt.x << ":" << evt.y);
		events.push_back(evt);
	}
	lw::g_gestrueMgr.onTouchEvent(events);
	
	if ( !lw::uiEvent(events) ){
		lw::TaskMgr::onTouchEvent(events);
	}
	lw::g_gestrueMgr.main();
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event{
	std::vector<lw::TouchEvent> events;
	for ( UITouch* touch in touches ){
		lw::TouchEvent evt;
		evt.type = lw::TouchEvent::UNTOUCH;
		CGPoint pt = [touch locationInView:self];
		CGPoint prevPt = [touch previousLocationInView:self];
		transXY(evt, pt, prevPt);
		events.push_back(evt);
	}
	lw::g_gestrueMgr.onTouchEvent(events);
	
	if ( !lw::uiEvent(events) ){
		lw::TaskMgr::onTouchEvent(events);
	}
	lw::g_gestrueMgr.main();
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event{
	std::vector<lw::TouchEvent> events;
	for ( UITouch* touch in touches ){
		lw::TouchEvent evt;
		evt.type = lw::TouchEvent::UNTOUCH;
		CGPoint pt = [touch locationInView:self];
		CGPoint prevPt = [touch previousLocationInView:self];
		transXY(evt, pt, prevPt);
		events.push_back(evt);
	}
	lw::g_gestrueMgr.onTouchEvent(events);
	
	if ( !lw::uiEvent(events) ){
		lw::TaskMgr::onTouchEvent(events);
	}
	lw::g_gestrueMgr.
main();
}


@end
