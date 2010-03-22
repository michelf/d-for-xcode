
// D for Xcode: Compiler Specification for DMD
// Copyright (c) 2007-2010  Michel Fortin
//
// D for Xcode is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
//
// D for Xcode is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
// more details.
//
// You should have received a copy of the GNU General Public License
// along with D for Xcode; if not, write to the Free Software Foundation, 
// Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.

#import <XCPSpecifications.h>

@interface DXCompilerSpecificationDMD : XCCompilerSpecification {
	NSString *cachedVersion;
}

+ (void) initialize;

- (id)initWithPropertyListDictionary:(id)plist;
- (void)dealloc;

- (NSArray *)importedFilesForPath:(NSString *)path ensureFilesExist:(BOOL)ensure inTargetBuildContext:(PBXTargetBuildContext *)context;

- (NSArray *)computeDependenciesForInputFile:(NSString *)input ofType:(PBXFileType*)type variant:(NSString *)variant architecture:(NSString *)arch outputDirectory:(NSString *)outputDir inTargetBuildContext:(PBXTargetBuildContext *)context;

- (NSString*)versionString;

- (NSString*)name;
- (NSString*)description;

@end
