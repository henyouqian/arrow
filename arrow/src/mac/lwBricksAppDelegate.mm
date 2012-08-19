//
//  dungeonHeroAppDelegate.m
//  dungeonHero
//
//  Created by ksh_mobile on 09/09/03.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "lwBricksAppDelegate.h"
#import "EAGLView.h"

#include "app.h"


@implementation lwBricksAppDelegate

@synthesize window;
@synthesize glView;

- (void)applicationDidFinishLaunching:(UIApplication *)application {
	glView.animationInterval = 1.0 / GAME_FPS;
	[glView startAnimation];
	
	appInit();
}

- (void)applicationWillTerminate:(UIApplication *)application {
	appQuit();
}


- (void)applicationWillResignActive:(UIApplication *)application {
	glView.animationInterval = 1.0 / 5.0;
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
	glView.animationInterval = 1.0 / GAME_FPS;
}


- (void)dealloc {
	[window release];
	[glView release];
	[super dealloc];
}

@end
