/*
 * << Haru Free PDF Library 2.0.0 >> -- font_demo.c
 *
 * Copyright (c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "hpdf.h"

jmp_buf env;

void
error_handler (HPDF_STATUS   error_no,
               HPDF_STATUS   detail_no,
               void         *user_data)
{
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
                (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

const char *font_list[] = {
    "Courier",
    "Courier-Bold",
    "Courier-Oblique",
    "Courier-BoldOblique",
    "Helvetica",
    "Helvetica-Bold",
    "Helvetica-Oblique",
    "Helvetica-BoldOblique",
    "Times-Roman",
    "Times-Bold",
    "Times-Italic",
    "Times-BoldItalic",
    "Symbol",
    "ZapfDingbats",
    NULL
};

int main (int argc, char **argv)
{
    const char *page_title = "Font Demo";
    HPDF_Doc  pdf;
    char fname[256];
    HPDF_Page page;
    HPDF_Font def_font;
    HPDF_REAL tw;
    HPDF_REAL height;
    HPDF_REAL width;
    HPDF_UINT i;

    strcpy (fname, argv[0]);
    strcat (fname, ".pdf");

	printf("Start file: %s\n", fname);

    pdf = HPDF_New (NULL, NULL);
    if (!pdf) {
        printf ("error: cannot create PdfDoc object\n");
        return 1;
    }

    if (setjmp(env)) {
        HPDF_Free (pdf);
        return 1;
    }

    /* Add a new page object. */
    page = HPDF_AddPage (pdf);

	printf("run 1 \n");

    height = HPDF_Page_GetHeight (page);
    width = HPDF_Page_GetWidth (page);
	printf("run 1 \n");


    /* Print the lines of the page. */
    HPDF_Page_SetLineWidth (page, 1);
    HPDF_Page_Rectangle (page, 50, 50, width - 100, height - 110);
    HPDF_Page_Stroke (page);
	printf("run 1 \n");

    /* Print the title of the page (with positioning center). */
    def_font = HPDF_GetFont (pdf, "Helvetica", NULL);

	printf("run def_font \n");


    HPDF_Page_SetFontAndSize (page, def_font, 24);

	printf("run font size \n");


    tw = HPDF_Page_TextWidth (page, page_title);
    HPDF_Page_BeginText (page);
    HPDF_Page_TextOut (page, (width - tw) / 2, height - 50, page_title);
    HPDF_Page_EndText (page);

	printf("run 2 \n");


    /* output subtitle. */
    HPDF_Page_BeginText (page);
    HPDF_Page_SetFontAndSize (page, def_font, 16);
    HPDF_Page_TextOut (page, 60, height - 80, "<Standerd Type1 fonts samples>");
    HPDF_Page_EndText (page);

    HPDF_Page_BeginText (page);
    HPDF_Page_MoveTextPos (page, 60, height - 105);

	printf("start font loop\n");

    i = 0;
    while (font_list[i]) {
        const char* samp_text = "abcdefgABCDEFG12345!#$%&+-@?";
        HPDF_Font font = HPDF_GetFont (pdf, font_list[i], NULL);

        /* print a label of text */
        HPDF_Page_SetFontAndSize (page, def_font, 9);
        HPDF_Page_ShowText (page, font_list[i]);
        HPDF_Page_MoveTextPos (page, 0, -18);

        /* print a sample text. */
        HPDF_Page_SetFontAndSize (page, font, 20);
        HPDF_Page_ShowText (page, samp_text);
        HPDF_Page_MoveTextPos (page, 0, -20);

        i++;
    }

	printf("end font loop\n");

    HPDF_Page_EndText (page);

	printf("end text\n");

    HPDF_SaveToFile (pdf, fname);

	printf("save file \n");



    /* clean up */
    HPDF_Free (pdf);

	printf("Success\n");

    return 0;
}

