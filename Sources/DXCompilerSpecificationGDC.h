//
//  DXGDCCompilerSpecification.h
//  D for Xcode
//
//  Created by Michel Fortin on 2007-09-28.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <XCPSpecifications.h>

@interface DXCompilerSpecificationGDC : XCCompilerSpecification {

}

+ (void) initialize;

- (NSArray *)importedFilesForPath:(NSString *)path ensureFilesExist:(BOOL)ensure inTargetBuildContext:(PBXTargetBuildContext *)context;

- (NSArray *)computeDependenciesForInputFile:(NSString *)input ofType:(PBXFileType*)type variant:(NSString *)variant architecture:(NSString *)arch outputDirectory:(NSString *)outputDir inTargetBuildContext:(PBXTargetBuildContext *)context;

@end

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * Determine file dependencies for input source.
 */
NSSet *DXSourceDependenciesForPath(NSString *input);

#if defined(__cplusplus)
}
#endif
