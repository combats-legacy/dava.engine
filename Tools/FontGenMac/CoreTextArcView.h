/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

/*
 
 File: CoreTextArcView.h
 
 Abstract: Declares the interface for the CoreTextArcView custom NSView 
 class that uses CoreText to draw text on a curve.
 
 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
 Apple Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Inc. 
 may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2008 Apple Inc. All Rights Reserved.
 
 */ 

#import <Cocoa/Cocoa.h>
#import "CharacterTablePanel.h"

@interface CoreTextArcView : NSView
{
@private
    NSFont* _font;
    NSString* _string;
    CGFloat _radius;

    struct
    {
        unsigned int showsGlyphBounds : 1;
        unsigned int showsLineMetrics : 1;
        unsigned int dimsSubstitutedGlyphs : 1;
        unsigned int reserved : 28;
    } _flags;

    NSString* symbolTable;

    CGFloat distanceTable[2048][2048];
    CGFloat kerningTable[2048][2048];
    CGFloat kerningBaseShift[2048];
    signed char fixedKerningTable[2048][2048];
    int kerningPairCount;
    Float32 defaultShiftValue;
    Float32 fontAscent;
    Float32 fontDescent;
    Float32 fontLeading;
    Float32 fontXHeight;
    Float32 fontWidth;
    Float32 fontHeight;

    CGContextRef symbolTableContext;
    CGImageRef symbolTableImage;

    CGRect symbolTypoBounds[2048];
    CGRect symbolBounds[2048];

    CGFloat maxSymbolWidth;

    float leftRightPadding;
    float topBottomPadding;
    float exportWidth;
    float exportHeight;

    bool isPrecisionKerning;

    BOOL enableShadow;

    IBOutlet CharacterTablePanel* characterTablePanel;
}

@property(retain, nonatomic) NSFont* font;
@property(retain, nonatomic) NSString* string;
@property(readonly, nonatomic) NSAttributedString* attributedString;
@property(assign, nonatomic) CGFloat radius;
@property(nonatomic) BOOL showsGlyphBounds;
@property(nonatomic) BOOL showsLineMetrics;
@property(nonatomic) BOOL dimsSubstitutedGlyphs;
@property(nonatomic) BOOL enableShadow;

- (void)exportStringToContext:(NSString*)exportString context:(CGContextRef)context export:(bool) export;
- (void)exportFontToFile;
- (void)extractKerningValues;
- (void)setSymbolTable:(NSString*)newSymbolTable;
@end
