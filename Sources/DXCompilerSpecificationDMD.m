
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

#import "DXCompilerSpecificationDMD.h"
#import "DXDependencyScanner.h"

#import "XCPBuildSystem.h"
#import "XCPDependencyGraph.h"
#import "XCPSupport.h"


@implementation DXCompilerSpecificationDMD

+ (void)initialize {
	PBXFileType *type = (PBXFileType*)[PBXFileType specificationForIdentifier:@"sourcecode.d"];
	XCCompilerSpecification *spec = (XCCompilerSpecification*)[XCCompilerSpecification specificationForIdentifier:@"com.michelf.dxcode.dmd"];
	[PBXTargetBuildContext activateImportedFileType:type withCompiler:spec];
}

- (id)initWithPropertyListDictionary:(id)plist {
	cachedVersion = 0;
	return [super initWithPropertyListDictionary:plist];
}

- (void)dealloc {
	[cachedVersion release];
	[super dealloc];
}

- (NSArray *)importedFilesForPath:(NSString *)path ensureFilesExist:(BOOL)ensure inTargetBuildContext:(PBXTargetBuildContext *)context
{
	//	NSString* outputDir = [context expandedValueForString:@"$(OBJFILES_DIR_$(variant))/$(arch)"];
	XCDependencyNode* inputNode = [context dependencyNodeForName:path createIfNeeded:YES];
	
	NSSet *filenames = DXSourceDependenciesForPath([NSString stringWithContentsOfFile:path]);
	NSMutableArray *imported = [NSMutableArray arrayWithCapacity:[filenames count]];
	
	NSEnumerator *e = [filenames objectEnumerator];
	NSString *filename;
	while (filename = [e nextObject]) {
		NSString *filepath = [context absolutePathForPath:filename];
		XCDependencyNode *node = [context dependencyNodeForName:filepath createIfNeeded:YES];
		[node setDontCareIfExists:YES];
		[inputNode addIncludedNode:node];
		[imported addObject:filename];
		
		//		if ([context expandedBooleanValueForString:@"$(GDC_GENERATE_INTERFACE_FILES)"] &&
		//			[filepath hasSuffix:@".d"])
		//		{
		//			NSString *objRoot = [context expandedValueForString:@"$(OBJROOT)"];
		//			NSString *interfaceDir = [objRoot stringByAppendingPathComponent:@"dinterface"];
		//			NSString *interfaceFile = [interfaceDir stringByAppendingPathComponent:[filepath stringByAppendingPathExtension:@"di"]];
		//			
		//			node = [context dependencyNodeForName:interfaceFile createIfNeeded:YES];
		//			[node setDontCareIfExists:YES];
		//			[inputNode addIncludedNode:node];
		//			[imported addObject:filename];
		//		}
	}
	
	return imported;
}


- (NSArray *)computeDependenciesForInputFile:(NSString *)input ofType:(PBXFileType*)type variant:(NSString *)variant architecture:(NSString *)arch outputDirectory:(NSString *)outputDir inTargetBuildContext:(PBXTargetBuildContext *)context
{
	// compute input file path
	input = [context expandedValueForString:input];
	
	// compute output file path
	NSString *basePath = [input stringByDeletingPathExtension];
	NSString *relativePath = [context naturalPathForPath:basePath];
	NSString *baseName = [relativePath stringByReplacingCharacter:'/' withCharacter:'.'];
	NSString *output = [baseName stringByAppendingPathExtension:@"o"];
	output = [outputDir stringByAppendingPathComponent:output];
	output = [context expandedValueForString:output];
	
	// create dependency nodes 
	XCDependencyNode *outputNode = [context dependencyNodeForName:output createIfNeeded:YES];
	XCDependencyNode *inputNode = [context dependencyNodeForName:input createIfNeeded:YES];
	
	// create compiler command
	XCDependencyCommand *dep = [context
		createCommandWithRuleInfo:[NSArray arrayWithObjects:@"Compile", [context naturalPathForPath:input],nil]
		commandPath:[context expandedValueForString:[self path]]
		arguments:nil
		forNode:outputNode];
	[dep setToolSpecification:self];
	[dep addArgumentsFromArray:[self commandLineForAutogeneratedOptionsInTargetBuildContext:context]];
	[dep addArgumentsFromArray:[[context expandedValueForString:@"$(build_file_compiler_flags)"] arrayByParsingAsStringList]];
	
	// Need to handle this flag programatically to avoid clashing with zerolink.
//	if([context expandedBooleanValueForString:@"$(GDC_DYNAMIC_NO_PIC)"]) {
//		if(![context expandedBooleanValueForString:@"$(ZERO_LINK)"]) {
//			[dep addArgument:@"-mdynamic-no-pic"];
//		}
//	}
	
	[dep addArgument:@"-c"];
	[dep addArgument:[@"-of" stringByAppendingString:output]];
	[dep addArgument:input];
	
	// avoid DMD outputing relative path for errors by changing the current dir
	[dep addArgument:[@"-I" stringByAppendingString:[dep workingDirectoryPath]]];
	[dep setWorkingDirectoryPath:@"/"];
	
	// Create dependency rules (must be done after dependency command creation)
	[outputNode addDependedNode:inputNode];
	
	// Tell Xcode to use DMD as the linker.
	NSString *v = [self versionString];
	if ([v length] && [v characterAtIndex:0] < '2')
		[context setStringValue:@"com.michelf.dxcode.dmd1.linker" forDynamicSetting:@"compiler_mandated_linker"];
	else
		[context setStringValue:@"com.michelf.dxcode.dmd2.linker" forDynamicSetting:@"compiler_mandated_linker"];
	// This fixes link problem for Xcode 3.1.
	[context setStringValue:@"/usr/bin/gcc" forDynamicSetting:@"gcc_compiler_driver_for_linking"];
	
	//	if ([context expandedBooleanValueForString:@"$(GDC_GENERATE_INTERFACE_FILES)"]) {
	//		NSString *objRoot = [context expandedValueForString:@"$(OBJROOT)"];
	//		NSString *interfaceDir = [objRoot stringByAppendingPathComponent:@"dinterface"];
	//		[dep addArgument:@"-I"];
	//		[dep addArgument:interfaceDir];
	//		
	//		NSString *interfaceFile = [interfaceDir stringByAppendingPathComponent:[relativePath stringByAppendingPathExtension:@"di"]];
	//		[dep addArgument:[NSString stringWithFormat:@"-fintfc-file=%@", interfaceFile]];
	//	}
	
	// update source <-> output links
	[context setCompiledFilePath:output forSourceFilePath:input];
	
	// set output objects
	return [NSArray arrayWithObject:outputNode];
}

- (NSString*)versionString {
	if (!cachedVersion) {
		// run executable and extract version number from first line.
		FILE *output = popen([[self executablePath] cString], "r");
		if (output) {
			char firstLine[48];
			if (fgets(firstLine, 47, output)) {
				// First line expected like "Digital Mars D Compiler v1.x"
				// skip to last word
				char *startOfLastWord = firstLine;
				char *cur = firstLine;
				while (*cur != '\0') {
					if (*cur == ' ')
						startOfLastWord = cur+1;
					else if (*cur == '\n')
						*cur = '\0';
					++cur;
				}
				// skip 'v' before version.
				if (*startOfLastWord == 'v')
					++startOfLastWord;
				cachedVersion = [[NSString alloc] initWithCString:startOfLastWord];
			} else
				cachedVersion = @"n/a";
			pclose(output);
		} else
			cachedVersion = @"n/a";
	}
	return cachedVersion;
}

- (NSString*)stringByAppendingVersionTo:(NSString *)str {
	return [str stringByAppendingFormat:@" (%@)", [self versionString]];
}

- (NSString*)name {
	return [self stringByAppendingVersionTo:[super name]];
}

- (NSString*)description {
	return [self stringByAppendingVersionTo:[super description]];
}

@end
