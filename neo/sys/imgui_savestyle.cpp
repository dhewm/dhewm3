/*
 * Functions to read/write a Dear ImGui style (ImGuiStyle) from/to a (ini-like)
 * textfile, or generating C++ code for it.
 *
 * Hosted at https://github.com/DanielGibson/Snippets/
 *
 * Written for/tested with Dear ImGui v1.91.4 (incl. Docking Branch)
 *
 * If anything changes in struct ImGuiStyle or enum ImGuiCol_, this code should detect it
 * during compilation and give (hopefully) helpful errors with static_assert() (always check and
 * fix the first error that occurs first, the remaining ones might be caused by the first).
 *
 * Released under the same license as Dear ImGui:
 *
 *   The MIT License (MIT)
 *
 *   Copyright (c) 2024 Daniel Gibson
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 */

#include "imgui.h"
#include <stdio.h>

#if 0 // this is basically the header, just copy it to wherever you want to use these functions
namespace DG {
	// writes the given ImGuiStyle to the given filename (opened with fopen())
	// returns true on success, false if opening the file failed
	extern bool WriteImGuiStyle( const ImGuiStyle& style, const char* filename );

	// reads the the given filename (opened with fopen())
	// and sets the given ImGuiStyle accordingly.
	// if any attributes/colors/behaviors are missing the the file,
	// they are not modified in style, so it probably makes sense to initialize
	// style to a sane default before calling this function.
	// returns true on success, false if opening the file failed
	extern bool ReadImGuiStyle( ImGuiStyle& style, const char* filename );

	// generate C++ code that replicates the given style into a text buffer
	// (that you can write to a file or set the clipboard from or whatever)
	// if refStyle is set, only differences in style compared to refStyle are written
	extern ImGuiTextBuffer WriteImGuiStyleToCode( const ImGuiStyle& style, const ImGuiStyle* refStyle = nullptr );
} //namespace
#endif

// if you want to use another namespace than DG, modify this #define
// (either here or through a compiler argumenet like -DDG_IMSAVESTYLE_NAMESPACE=horst)
#ifndef DG_IMSAVESTYLE_NAMESPACE
  #define DG_IMSAVESTYLE_NAMESPACE DG
#endif

// #define your own DG_IMSAVESTYLE_FOPEN if you want to use an alternative to
// standard fopen(), for example one that supports UTF-8 filenames on Windows
#ifndef DG_IMSAVESTYLE_FOPEN
  #define DG_IMSAVESTYLE_FOPEN fopen
#endif

#undef warnPrintf
// TODO: maybe use your own logging system instead of fprintf() to stderr
#define warnPrintf(...)  fprintf( stderr, "Warning: " __VA_ARGS__ )

#ifdef IMGUI_HAS_DOCK
  // some members of struct ImGuiStyle and enum ImGuiCol_ only exist in the Docking branch..
  #define DGIMST_ENABLE_IF_DOCKING(x) x
#else
  // .. so disable them in the regular branch
  #define DGIMST_ENABLE_IF_DOCKING(x)
#endif

// Note: The trick I'm using with these #defines below is called "X Macro"
//       see https://en.wikipedia.org/wiki/X_macro (except I'm not calling the "entries" X,
//       and have more then one kind of entry per table)
// If you're wondering about the D3_ prefix on the macros, I originally wrote this for dhewm3.

// this table contains all members of struct ImGuiStyle that come before the Colors array
// when updating Dear ImGui, this might have to be adjusted for members that have been added/removed/renamed
#define D3_IMSTYLE_ATTRS \
    D3_IMATTR_FLOAT( FontSizeBase                     ) \
    D3_IMATTR_FLOAT( FontScaleMain                    ) \
    D3_IMATTR_FLOAT( FontScaleDpi                     ) \
    D3_IMATTR_FLOAT( Alpha                            ) \
    D3_IMATTR_FLOAT( DisabledAlpha                    ) \
    D3_IMATTR_VEC2(  WindowPadding                    ) \
    D3_IMATTR_FLOAT( WindowRounding                   ) \
    D3_IMATTR_FLOAT( WindowBorderSize                 ) \
    D3_IMATTR_FLOAT( WindowBorderHoverPadding         ) \
    D3_IMATTR_VEC2(  WindowMinSize                    ) \
    D3_IMATTR_VEC2(  WindowTitleAlign                 ) \
    D3_IMATTR_DIR(   WindowMenuButtonPosition         ) \
    D3_IMATTR_FLOAT( ChildRounding                    ) \
    D3_IMATTR_FLOAT( ChildBorderSize                  ) \
    D3_IMATTR_FLOAT( PopupRounding                    ) \
    D3_IMATTR_FLOAT( PopupBorderSize                  ) \
    D3_IMATTR_VEC2(  FramePadding                     ) \
    D3_IMATTR_FLOAT( FrameRounding                    ) \
    D3_IMATTR_FLOAT( FrameBorderSize                  ) \
    D3_IMATTR_VEC2(  ItemSpacing                      ) \
    D3_IMATTR_VEC2(  ItemInnerSpacing                 ) \
    D3_IMATTR_VEC2(  CellPadding                      ) \
    D3_IMATTR_VEC2(  TouchExtraPadding                ) \
    D3_IMATTR_FLOAT( IndentSpacing                    ) \
    D3_IMATTR_FLOAT( ColumnsMinSpacing                ) \
    D3_IMATTR_FLOAT( ScrollbarSize                    ) \
    D3_IMATTR_FLOAT( ScrollbarRounding                ) \
    D3_IMATTR_FLOAT( ScrollbarPadding                 ) \
    D3_IMATTR_FLOAT( GrabMinSize                      ) \
    D3_IMATTR_FLOAT( GrabRounding                     ) \
    D3_IMATTR_FLOAT( LogSliderDeadzone                ) \
    D3_IMATTR_FLOAT( ImageBorderSize                  ) \
    D3_IMATTR_FLOAT( TabRounding                      ) \
    D3_IMATTR_FLOAT( TabBorderSize                    ) \
    D3_IMATTR_FLOAT( TabMinWidthBase                  ) \
    D3_IMATTR_FLOAT( TabMinWidthShrink                ) \
    D3_IMATTR_FLOAT( TabCloseButtonMinWidthSelected   ) \
    D3_IMATTR_FLOAT( TabCloseButtonMinWidthUnselected ) \
    D3_IMATTR_FLOAT( TabBarBorderSize                 ) \
    D3_IMATTR_FLOAT( TabBarOverlineSize               ) \
    D3_IMATTR_FLOAT( TableAngledHeadersAngle          ) \
    D3_IMATTR_VEC2(  TableAngledHeadersTextAlign      ) \
    D3_IMATTR_INT(   TreeLinesFlags                   ) \
    D3_IMATTR_FLOAT( TreeLinesSize                    ) \
    D3_IMATTR_FLOAT( TreeLinesRounding                ) \
    D3_IMATTR_FLOAT( DragDropTargetRounding           ) \
    D3_IMATTR_FLOAT( DragDropTargetBorderSize         ) \
    D3_IMATTR_FLOAT( DragDropTargetPadding            ) \
    D3_IMATTR_DIR(   ColorButtonPosition              ) \
    D3_IMATTR_VEC2(  ButtonTextAlign                  ) \
    D3_IMATTR_VEC2(  SelectableTextAlign              ) \
    D3_IMATTR_FLOAT( SeparatorTextBorderSize          ) \
    D3_IMATTR_VEC2(  SeparatorTextAlign               ) \
    D3_IMATTR_VEC2(  SeparatorTextPadding             ) \
    D3_IMATTR_VEC2(  DisplayWindowPadding             ) \
    D3_IMATTR_VEC2(  DisplaySafeAreaPadding           ) \
    D3_IMATTR_FLOAT( MouseCursorScale                 ) \
    D3_IMATTR_BOOL(  AntiAliasedLines                 ) \
    D3_IMATTR_BOOL(  AntiAliasedLinesUseTex           ) \
    D3_IMATTR_BOOL(  AntiAliasedFill                  ) \
    D3_IMATTR_FLOAT( CurveTessellationTol             ) \
    D3_IMATTR_FLOAT( CircleTessellationMaxError       ) \
    DGIMST_ENABLE_IF_DOCKING( D3_IMATTR_FLOAT( DockingSeparatorSize ) )

// this table contains the members of struct ImGuiStyle that come after the Colors array
// when updating Dear ImGui, this might have to be adjusted for members that have been added/removed/renamed
#define D3_IMSTYLE_BEHAVIORS \
	D3_IMATTR_FLOAT( HoverStationaryDelay        ) \
	D3_IMATTR_FLOAT( HoverDelayShort             ) \
	D3_IMATTR_FLOAT( HoverDelayNormal            ) \
	D3_IMATTR_INT(   HoverFlagsForTooltipMouse   ) \
	D3_IMATTR_INT(   HoverFlagsForTooltipNav     ) \
	D3_IMATTR_FLOAT( _MainScale                  ) \
	D3_IMATTR_FLOAT( _NextFrameFontSizeBase      )

// this table contains one entry for every value of enum ImGuiCol_ (except for ImGuiCol_COUNT)
// as you can see, the "ImGuiCol_" prefix is left out in this table,
// it's added programatically when needed
// when updating Dear ImGui, this might have to be adjusted for enum members that have been added/removed/renamed
#define D3_IMSTYLE_COLORS \
	D3_IMSTYLE_COLOR( Text                      ) \
	D3_IMSTYLE_COLOR( TextDisabled              ) \
	D3_IMSTYLE_COLOR( WindowBg                  ) \
	D3_IMSTYLE_COLOR( ChildBg                   ) \
	D3_IMSTYLE_COLOR( PopupBg                   ) \
	D3_IMSTYLE_COLOR( Border                    ) \
	D3_IMSTYLE_COLOR( BorderShadow              ) \
	D3_IMSTYLE_COLOR( FrameBg                   ) \
	D3_IMSTYLE_COLOR( FrameBgHovered            ) \
	D3_IMSTYLE_COLOR( FrameBgActive             ) \
	D3_IMSTYLE_COLOR( TitleBg                   ) \
	D3_IMSTYLE_COLOR( TitleBgActive             ) \
	D3_IMSTYLE_COLOR( TitleBgCollapsed          ) \
	D3_IMSTYLE_COLOR( MenuBarBg                 ) \
	D3_IMSTYLE_COLOR( ScrollbarBg               ) \
	D3_IMSTYLE_COLOR( ScrollbarGrab             ) \
	D3_IMSTYLE_COLOR( ScrollbarGrabHovered      ) \
	D3_IMSTYLE_COLOR( ScrollbarGrabActive       ) \
	D3_IMSTYLE_COLOR( CheckMark                 ) \
	D3_IMSTYLE_COLOR( SliderGrab                ) \
	D3_IMSTYLE_COLOR( SliderGrabActive          ) \
	D3_IMSTYLE_COLOR( Button                    ) \
	D3_IMSTYLE_COLOR( ButtonHovered             ) \
	D3_IMSTYLE_COLOR( ButtonActive              ) \
	D3_IMSTYLE_COLOR( Header                    ) \
	D3_IMSTYLE_COLOR( HeaderHovered             ) \
	D3_IMSTYLE_COLOR( HeaderActive              ) \
	D3_IMSTYLE_COLOR( Separator                 ) \
	D3_IMSTYLE_COLOR( SeparatorHovered          ) \
	D3_IMSTYLE_COLOR( SeparatorActive           ) \
	D3_IMSTYLE_COLOR( ResizeGrip                ) \
	D3_IMSTYLE_COLOR( ResizeGripHovered         ) \
	D3_IMSTYLE_COLOR( ResizeGripActive          ) \
	D3_IMSTYLE_COLOR( InputTextCursor           ) \
	D3_IMSTYLE_COLOR( TabHovered                ) \
	D3_IMSTYLE_COLOR( Tab                       ) \
	D3_IMSTYLE_COLOR( TabSelected               ) \
	D3_IMSTYLE_COLOR( TabSelectedOverline       ) \
	D3_IMSTYLE_COLOR( TabDimmed                 ) \
	D3_IMSTYLE_COLOR( TabDimmedSelected         ) \
	D3_IMSTYLE_COLOR( TabDimmedSelectedOverline ) \
	D3_IMSTYLE_COLOR( PlotLines                 ) \
	D3_IMSTYLE_COLOR( PlotLinesHovered          ) \
	D3_IMSTYLE_COLOR( PlotHistogram             ) \
	D3_IMSTYLE_COLOR( PlotHistogramHovered      ) \
	D3_IMSTYLE_COLOR( TableHeaderBg             ) \
	D3_IMSTYLE_COLOR( TableBorderStrong         ) \
	D3_IMSTYLE_COLOR( TableBorderLight          ) \
	D3_IMSTYLE_COLOR( TableRowBg                ) \
	D3_IMSTYLE_COLOR( TableRowBgAlt             ) \
	D3_IMSTYLE_COLOR( TextLink                  ) \
	D3_IMSTYLE_COLOR( TextSelectedBg            ) \
	D3_IMSTYLE_COLOR( TreeLines                 ) \
	D3_IMSTYLE_COLOR( DragDropTarget            ) \
	D3_IMSTYLE_COLOR( DragDropTargetBg          ) \
	D3_IMSTYLE_COLOR( UnsavedMarker             ) \
	D3_IMSTYLE_COLOR( NavCursor                 ) \
	D3_IMSTYLE_COLOR( NavWindowingHighlight     ) \
	D3_IMSTYLE_COLOR( NavWindowingDimBg         ) \
	D3_IMSTYLE_COLOR( ModalWindowDimBg          ) \
	DGIMST_ENABLE_IF_DOCKING( D3_IMSTYLE_COLOR( DockingPreview ) ) \
	DGIMST_ENABLE_IF_DOCKING( D3_IMSTYLE_COLOR( DockingEmptyBg ) )

static inline char* skipWhitespace( const char* s ) {
	while( *s == ' ' || *s == '\t' )
		++s;
	return (char*)s;
}

#define D3_IMATTR_FLOAT( NAME ) \
	if ( sscanf( line, #NAME " = %f", &f1 ) == 1 ) { \
		s . NAME = f1; \
		return; \
	}

#define D3_IMATTR_VEC2( NAME )  \
	if ( sscanf( line, #NAME " = %f , %f", &f1, &f2 ) == 2 ) { \
		s . NAME .x = f1; s . NAME .y = f2; \
		return; \
	}

#define D3_IMATTR_INT( NAME )   \
	if ( sscanf( line, #NAME " = %d", &i ) == 1 ) { \
		s . NAME = i; \
		return; \
	}

#define D3_IMATTR_DIR( NAME )   \
	if ( sscanf( line, #NAME " = %d", &i ) == 1 ) { \
		s . NAME = (ImGuiDir)i; \
		return; \
	}

#define D3_IMATTR_BOOL( NAME )  \
	if ( sscanf( line, #NAME " = %d", &i ) == 1 ) { \
		s . NAME = ( i != 0 ); \
		return; \
	}


static void parseStyleLine( ImGuiStyle& s, const char* line )
{
	float f1=0, f2=0;
	int i=0;

	// with the D3_IMATTR_* #defines above, the following
	// `D3_IMSTYLE_ATTRS` line turns into:
	//  if ( sscanf( line, "Alpha = %f", &f1 ) == 1 ) {
	//      s . Alpha = f1;
	//      return;
	//  }
	//  if ( sscanf( line, "DisabledAlpha = %f", &f1 ) == 1 ) {
	//      s . DisabledAlpha = f1;
	//      return;
	//  }
	//  if ( sscanf( line, "WindowPadding = %f , %f", &f1, &f2 ) == 2 ) {
	//      s . WindowPadding .x = f1; s . WindowPadding .y = f2;
	//      return;
	//  }
	// etc

	D3_IMSTYLE_ATTRS

	// NOTE: if a member is renamed, for backwards-compatibility you could add code here like
	// if ( sscanf( line, "OldName = %f", %f1 ) == 1 ) {
	//     s.NewName = f;
	//     return;
	// }

	warnPrintf( "Invalid line in ImGui style under [style] section: '%s'\n", line );
}

static void parseBehaviorLine( ImGuiStyle& s, const char* line )
{
	float f1=0, f2=0;
	(void)f2; // currently unused in behavior section
	int i=0;

	D3_IMSTYLE_BEHAVIORS

	// NOTE: same thing for backwards-compatbility as in parseStyleLine applies here

	warnPrintf( "Invalid line in ImGui style under [behaviors] section: '%s'\n", line );
}

#undef D3_IMATTR_FLOAT
#undef D3_IMATTR_VEC2
#undef D3_IMATTR_INT
#undef D3_IMATTR_DIR
#undef D3_IMATTR_BOOL

namespace {
// for renamed colors
struct ImGuiColorBackwardCompat {
	const char* oldColorStr;
	enum ImGuiCol_ newColorVal;
};

static struct ImGuiColorBackwardCompat backwardCompatColorMappings[] = {
	{ "TabActive",                    ImGuiCol_TabSelected },       // renamed in 1.90.9
	{ "TabUnfocused",                 ImGuiCol_TabDimmed },         // renamed in 1.90.9
	{ "TabUnfocusedActive",           ImGuiCol_TabDimmedSelected }, // renamed in 1.90.9
	{ "NavHighlight",                 ImGuiCol_NavCursor },      // renamed in 1.91.4
};

} //anon namespace

static void parseColorLine( ImGuiStyle& s, const char* line )
{
	ImVec4 c;

#define D3_IMSTYLE_COLOR( NAME ) \
	if ( sscanf( line, #NAME " = %f , %f , %f , %f", &c.x, &c.y, &c.z, &c.w) == 4 ) { \
		s.Colors[ ImGuiCol_ ## NAME ] = c; \
		return; \
	}

	D3_IMSTYLE_COLORS

#undef D3_IMSTYLE_COLOR

	for( const ImGuiColorBackwardCompat& bc : backwardCompatColorMappings ) {
		char matchString[64];
		snprintf( matchString, sizeof(matchString), "%s = %%f , %%f , %%f , %%f", bc.oldColorStr );

		if ( sscanf( line, matchString, &c.x, &c.y, &c.z, &c.w) == 4 ) {
			s.Colors[ bc.newColorVal ] = c;
			return;
		}
	}

	warnPrintf( "Invalid line in ImGui style under [colors] section: '%s'\n", line );
}

namespace DG_IMSAVESTYLE_NAMESPACE {

bool ReadImGuiStyle( ImGuiStyle& s, const char* filename )
{
	FILE* f = DG_IMSAVESTYLE_FOPEN( filename, "r" ); // TODO: "rt" on Windows?
	if ( f == nullptr ) {
		// Note: Turns out that printing warning here is annoying, there's nothing wrong with the
		//  user not having saved a style yet.. just return false, the caller can log if appropriate
		return false;
	}

	char lineBuf[256];

	int section = -1; // 0: style, 1: behaviors, 2: colors

	for ( char* line = fgets( lineBuf, sizeof(lineBuf), f );
		line != nullptr;
		line = fgets( lineBuf, sizeof(lineBuf), f ) )
	{
		// skip whitespace, if any
		line = skipWhitespace(line);
		if ( *line == '#' ) // skip comment lines
			continue;
		if ( *line == '[' ) { // start of a section
			const char* secStr = line+1; // skip '['
			secStr = skipWhitespace(secStr);
			// "[style]" "[behaviors]" "[colors]"
			if ( strncmp(secStr, "style", 5) == 0 ) {
				section = 0;
			} else if ( strncmp(secStr, "behaviors", 9) == 0 ) {
				section = 1;
			} else if ( strncmp(secStr, "colors", 6) == 0 ) {
				section = 2;
			} else {
				warnPrintf( "Invalid line that looks like a section in ImGui style: '%s'\n", line );
			}
			continue;
		}
		if ( *line == '\r' || *line == '\n' ) {
			continue; // empty line
		}
		if ( section == 0 ) {
			parseStyleLine( s, line );
		} else if ( section == 1 ) {
			parseBehaviorLine( s, line );
		} else if ( section == 2 ) {
			parseColorLine( s, line );
		} else {
			warnPrintf( "Invalid line in ImGui before start of any section: '%s'\n", line );
		}
	}

	return true;
}

bool WriteImGuiStyle( const ImGuiStyle& s, const char* filename ) {
	FILE* f = DG_IMSAVESTYLE_FOPEN( filename, "w" ); // TODO: "wt" on Windows?
	if ( f == nullptr ) {
		warnPrintf( "Couldn't open '%s' for writing\n", filename );
		return false;
	}

	fprintf( f, "[style]\n" );

#define D3_IMATTR_FLOAT( NAME ) \
	fprintf( f, "%s = %g\n", #NAME, s . NAME );
#define D3_IMATTR_VEC2( NAME ) \
	fprintf( f, "%s = %g, %g\n", #NAME, s . NAME . x, s . NAME . y );
#define D3_IMATTR_INT( NAME ) \
	fprintf( f, "%s = %d\n", #NAME, s . NAME );
#define D3_IMATTR_DIR( NAME ) \
	fprintf( f, "%s = %d\n", #NAME, s . NAME );
#define D3_IMATTR_BOOL( NAME ) \
	fprintf( f, "%s = %d\n", #NAME, (int)( s . NAME ) );

	// this (together with the D3_IMATTR_* defines in the previous lines)
	// expands the D3_IMSTYLE_ATTRS table to
	//  fprintf( f, "Alpha = %g\n", s . Alpha );
	//  fprintf( f, "DisabledAlpha = %g\n", s . DisabledAlpha );
	//  fprintf( f, "WindowPadding = %g %g\n", s . WindowPadding . x, s . WindowPadding . y );
	// etc
	D3_IMSTYLE_ATTRS

	fprintf( f, "\n[behaviors]\n" );

	// same for behaviors
	D3_IMSTYLE_BEHAVIORS

#undef D3_IMATTR_FLOAT
#undef D3_IMATTR_VEC2
#undef D3_IMATTR_INT
#undef D3_IMATTR_DIR
#undef D3_IMATTR_BOOL

	fprintf( f, "\n[colors]\n" );

#define D3_IMSTYLE_COLOR( NAME ) { \
		const ImVec4& c = s.Colors[ ImGuiCol_ ## NAME  ]; \
		fprintf( f, "%s = %g, %g, %g, %g\n", #NAME, c.x, c.y, c.z, c.w ); \
	}

	// this turns into
	//  { const ImVec4& c = s.Colors[ ImGuiCol_Text ]; fprintf( f, "Text = %g %g %g %g\n", c.x, c.y, c.z, c.w ); }
	//  { const ImVec4& c = s.Colors[ ImGuiCol_TextDisabled ]; fprintf( f, "TextDisabled = %g %g %g %g\n", c.x, c.y, c.z, c.w ); }
	// etc
	D3_IMSTYLE_COLORS

#undef D3_IMSTYLE_COLOR

	fprintf( f, "\n" );
	fflush( f );
	fclose( f );

	return true;
}

static inline int numSpaces( const char* name, int targetLen )
{
	int ret = targetLen - strlen(name);
	return ret > 0 ? ret : 0;
}

ImGuiTextBuffer WriteImGuiStyleToCode( const ImGuiStyle& s, const ImGuiStyle* refStyle )
{
	ImGuiTextBuffer ret;
	ret.reserve(6000);
	ret.append( "ImGuiStyle& style = ImGui::GetStyle();\n" );

#define D3_IMATTR_FLOAT( NAME ) \
	if ( refStyle == nullptr || s. NAME != refStyle-> NAME ) { \
		ret.appendf( "style.%s %*s= %g;\n", #NAME, numSpaces( #NAME , 27 ), "", s. NAME ); \
	}
#define D3_IMATTR_VEC2( NAME ) \
	if ( refStyle == nullptr || s. NAME .x != refStyle-> NAME .x || s. NAME .y != refStyle-> NAME .y ) { \
		ret.appendf( "style.%s %*s= ImVec2( %g, %g );\n", #NAME, numSpaces( #NAME , 27 ), "", s. NAME .x, s. NAME .y ); \
	}
#define D3_IMATTR_INT( NAME ) \
	if ( refStyle == nullptr || s. NAME != refStyle-> NAME ) { \
		ret.appendf( "style.%s = %d; // TODO: flag\n", #NAME, s. NAME ); \
	}
#define D3_IMATTR_DIR( NAME ) \
	if ( refStyle == nullptr || s. NAME != refStyle-> NAME ) { \
		ret.appendf( "style.%s = %d;\n", #NAME, s. NAME ); \
	}
#define D3_IMATTR_BOOL( NAME ) \
	if ( refStyle == nullptr || s. NAME != refStyle-> NAME ) { \
		ret.appendf( "style.%s %*s= %s;\n", #NAME, numSpaces( #NAME , 27 ), "", s. NAME ? "true" : "false" ); \
	}

	D3_IMSTYLE_ATTRS

	ret.append("\n");

	D3_IMSTYLE_BEHAVIORS

#undef D3_IMATTR_FLOAT
#undef D3_IMATTR_VEC2
#undef D3_IMATTR_INT
#undef D3_IMATTR_DIR
#undef D3_IMATTR_BOOL

	ret.append( "\nImVec4* colors = style.Colors;\n" );

	for ( ImGuiCol cIdx=0; cIdx < ImGuiCol_COUNT; ++cIdx ) {
		const ImVec4& c = s.Colors[cIdx];
		if ( refStyle != nullptr ) {
			const ImVec4& c2 = refStyle->Colors[cIdx];
			if ( c.x == c2.x && c.y == c2.y && c.z == c2.z && c.w == c2.w ) {
				// same colors, nothing to do
				continue;
			}
		}
		const char* name = ImGui::GetStyleColorName( cIdx );
		ret.appendf( "colors[ ImGuiCol_%s ]%*s = ImVec4( %g, %g, %g, %g );\n",
		             name, numSpaces( name , 21 ), "", c.x, c.y, c.z, c.w );
	}

	ret.append("\n");

	return ret;
}

} //namespace DG

// check correctness of the X macro tables above (detect when something is added/removed/renamed in struct ImGuiStyle or enum ImGuiCol_)
namespace {

// recreate enum ImGuiCol_ from the D3_IMSTYLE_COLORS table to compare its members
// so we can easily detect when (and where) one is added
enum D3CHECK_ImguiCol_ {

#define D3_IMSTYLE_COLOR( NAME )  D3CHECK_ImGuiCol_ ## NAME ,

	D3_IMSTYLE_COLORS

#undef D3_IMSTYLE_COLOR

	D3CHECK_ImGuiCol_COUNT
};

#define D3_IMSTYLE_COLOR( NAME ) \
	static_assert( (int) D3CHECK_ImGuiCol_ ## NAME == (int) ImGuiCol_ ## NAME, "Wrong value for ImGuiCol_" #NAME " - probably something was added to enum ImGuiCol_ before it? => => adjust D3_IMSTYLE_COLORS table above" );

D3_IMSTYLE_COLORS

#undef D3_IMSTYLE_COLOR

// if the following is the only failing static_assert, the new color was probably added to the end
static_assert( (int)D3CHECK_ImGuiCol_COUNT == (int)ImGuiCol_COUNT, "something was added or removed in enum ImGuiCol_ => adjust D3_IMSTYLE_COLORS table above" );

// recreate struct ImGuiStyle from the tables above and see if they're identical
// (this struct is only used for the static assertions below)
struct D3_ImGuiStyle_Check {

#define D3_IMATTR_FLOAT( NAME ) float NAME ;
#define D3_IMATTR_VEC2( NAME )  ImVec2 NAME ;
#define D3_IMATTR_INT( NAME )   int NAME ;
#define D3_IMATTR_DIR( NAME )   ImGuiDir NAME ;
#define D3_IMATTR_BOOL( NAME )  bool NAME ;

	// this expands to all the ImGuiStyle members, up to (excluding) Colors
	// exactly like in ImGuiStyle (except the pseudo-enums like ImGuiDir are plain ints here)
	D3_IMSTYLE_ATTRS

	ImVec4 Colors[ImGuiCol_COUNT];

	// just like the other members/attributes, expand the behaviors
	D3_IMSTYLE_BEHAVIORS

#undef D3_IMATTR_FLOAT
#undef D3_IMATTR_VEC2
#undef D3_IMATTR_INT
#undef D3_IMATTR_DIR
#undef D3_IMATTR_BOOL

};

template<class T, class U>
struct is_same {
	static constexpr bool value = false;
};

template<class T>
struct is_same<T, T> {
	static constexpr bool value = true;
};

#define D3_IMATTR_FLOAT( NAME ) \
	static_assert( offsetof( ImGuiStyle, NAME ) == offsetof( D3_ImGuiStyle_Check, NAME ), \
			"member " #NAME " not at expected offset - is the member before it missing from the list, or moved to another position?" ); \
	static_assert( is_same< decltype( ImGuiStyle :: NAME ), float >::value, "expected member " #NAME "to be a float - adjust the list!" );

#define D3_IMATTR_VEC2( NAME ) \
	static_assert( offsetof( ImGuiStyle, NAME ) == offsetof( D3_ImGuiStyle_Check, NAME ), \
			"member " #NAME " not at expected offset - is the member before it missing from the list, or moved to another position?" ); \
	static_assert( is_same< decltype( ImGuiStyle :: NAME ), ImVec2 >::value, "expected member " #NAME " to be an ImVec2 - adjust the list!" );

#define D3_IMATTR_INT( NAME ) \
	static_assert( offsetof( ImGuiStyle, NAME ) == offsetof( D3_ImGuiStyle_Check, NAME ), \
			"member " #NAME " not at expected offset - is the member before it missing from the list, or moved to another position?" ); \
	static_assert( is_same< decltype( ImGuiStyle :: NAME ), int >::value, "expected member " #NAME " to be an int - adjust the list!" );

#define D3_IMATTR_DIR( NAME ) \
	static_assert( offsetof( ImGuiStyle, NAME ) == offsetof( D3_ImGuiStyle_Check, NAME ), \
			"member " #NAME " not at expected offset - is the member before it missing from the list, or moved to another position?" ); \
	static_assert( is_same< decltype( ImGuiStyle :: NAME ), ImGuiDir >::value, "expected member " #NAME " to be an ImGuiDir - adjust the list!" );

#define D3_IMATTR_BOOL( NAME ) \
	static_assert( offsetof( ImGuiStyle, NAME ) == offsetof( D3_ImGuiStyle_Check, NAME ), \
			"member " #NAME " not at expected offset - is the member before it missing from the list, or moved to another position?" ); \
	static_assert( is_same< decltype( ImGuiStyle :: NAME ), bool >::value, "expected member " #NAME " to be a bool - adjust the list!" );

// expanding those static assertions for offset and type for all attributes and behaviors

D3_IMSTYLE_ATTRS

D3_IMSTYLE_BEHAVIORS

#undef D3_IMATTR_FLOAT
#undef D3_IMATTR_VEC2
#undef D3_IMATTR_INT
#undef D3_IMATTR_BOOL

static_assert( offsetof( ImGuiStyle, Colors ) == offsetof( D3_ImGuiStyle_Check, Colors ), "member Colors not at expected offset" );

// if all other static assertions passed and the following failed, probably a member was added at the end of the ImGuiStyle struct
static_assert( sizeof(D3_ImGuiStyle_Check) == sizeof(ImGuiStyle),
		"something seems to be missing or wrong type in D3_IMSTYLE_ATTRS or D3_IMSTYLE_BEHAVIORS" );

} //anon namespace
