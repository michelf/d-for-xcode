//
//  DXFileTypeSetup.m
//  D for Xcode
//
//  Created by Michel Fortin on 2009-07-11.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "DXFileTypeSetup.h"
#import "XCPSpecifications.h"
#import <objc/objc-class.h>
#import <objc/objc-runtime.h>

static BOOL DXIsXcode3OrLater() {
	const int xcode3version = 921;
	int version = [[[[NSBundle mainBundle] infoDictionary] valueForKey:@"CFBundleVersion"] intValue];
	return version >= xcode3version || version == 0;
}


@implementation PBXFileType (DXOverride)

+ (PBXFileType*)DX_fileTypeForPath:(NSString*)path getExtraFileProperties:(id*)props {
	// Priorize "sourcecode.d" over "sourcecode.dtrace" for the ".d" file extension.
	PBXFileType* originalType = [self DX_fileTypeForPath:path getExtraFileProperties:props];
	if ([path hasSuffix:@".d"])
	{
		PBXFileType *type = (PBXFileType*)[PBXFileType specificationForIdentifier:@"sourcecode.d"];
		if (type)
			return (PBXFileType*)[type loadedSpecification];
	}
	return originalType;
}

+ (PBXFileType *)DX_fileTypeForFileName:(NSString*)name posixPermissions:(unsigned)perms hfsTypeCode:(unsigned)type hfsCreatorCode:(unsigned)creator {
	// Priorize "sourcecode.d" over "sourcecode.dtrace" for the ".d" file extension.
	if ([name hasSuffix:@".d"])
	{
		PBXFileType *type = (PBXFileType*)[PBXFileType specificationForIdentifier:@"sourcecode.d"];
		if (type)
			return type;
	}
	return [self DX_fileTypeForFileName:name posixPermissions:perms hfsTypeCode:type hfsCreatorCode:creator];
}

@end


@implementation DXFileTypeSetup

+ (void)load {
	// Override fileTypeForFileName: on PBXFileType which will give priority to
	// sourcecode.d over sourcecode.dtrace for the .d file extension.
	if(DXIsXcode3OrLater()) {
		Class c = objc_getClass("PBXFileType");
		
		// Exchange method implementations
		{
			Method m = class_getClassMethod(c, @selector(fileTypeForFileName:posixPermissions:hfsTypeCode:hfsCreatorCode:));
			Method dxm = class_getClassMethod(c, @selector(DX_fileTypeForFileName:posixPermissions:hfsTypeCode:hfsCreatorCode:));
			IMP original = dxm->method_imp;
			dxm->method_imp = m->method_imp;
			m->method_imp = original;
		}
		{
			Method m = class_getClassMethod(c, @selector(fileTypeForPath:getExtraFileProperties:));
			Method dxm = class_getClassMethod(c, @selector(DX_fileTypeForPath:getExtraFileProperties:));
			IMP original = dxm->method_imp;
			dxm->method_imp = m->method_imp;
			m->method_imp = original;
		}
			
		// Clear Objective-C runtime cache
		if(c->cache->mask != 0)
			memset(c->cache->buckets, 0, (c->cache->mask+1)*sizeof(Method));
	}
}

@end
