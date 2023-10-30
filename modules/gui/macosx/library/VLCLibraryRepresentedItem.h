/*****************************************************************************
 * VLCLibraryRepresentedItem.h: MacOS X interface module
 *****************************************************************************
 * Copyright (C) 2023 VLC authors and VideoLAN
 *
 * Authors: Claudio Cambra <developer@claudiocambra.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#import <Cocoa/Cocoa.h>

#import <vlc_media_library.h>

NS_ASSUME_NONNULL_BEGIN

@protocol VLCMediaLibraryItemProtocol;

@interface VLCLibraryRepresentedItem : NSObject

@property (readonly) id<VLCMediaLibraryItemProtocol> item;
@property (readonly) id<VLCMediaLibraryItemProtocol> parentItem;
@property (readonly) enum vlc_ml_parent_type parentType;
@property (readonly) NSInteger itemIndexInParent;

- (instancetype)initWithItem:(const id<VLCMediaLibraryItemProtocol>)item
                  parentType:(const enum vlc_ml_parent_type)parentType;

- (void)play;
- (void)queue;
- (void)revealInFinder;
- (void)moveToTrash;

@end

NS_ASSUME_NONNULL_END