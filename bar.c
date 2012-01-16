// bar.c [ 0.3.1 ]

/* ************************** Status Bar *************************** */
void setup_status_bar() {
    int i;
    XGCValues values;

    show_bar = STATUS_BAR;
    logger(" \033[0;33mStatus Bar called ...");

    for(i=0;i<5;i++) {
        values.foreground = theme[i+4].color;
        values.line_width = 2;
        values.line_style = LineSolid;
        values.font = fontbar->fid;
        theme[i].gc = XCreateGC(dis, root, GCForeground|GCLineWidth|GCLineStyle|GCFont,&values);
    }

    sb_width = 0;
    for(i=0;i<DESKTOPS;i++) {
        sb_bar[i].width = XTextWidth(fontbar, sb_bar[i].label, strlen(sb_bar[i].label)+1);
        if(sb_bar[i].width > sb_width)
            sb_width = sb_bar[i].width;
    }
    sb_width += 4;
    if(sb_width < sb_height) sb_width = sb_height;
    sb_desks = (DESKTOPS*sb_width)+bdw;
}

void status_bar() {
    int i, y;

    if(topbar == 0) y = 0;
    else y = sh+bdw;
    for(i=0;i<DESKTOPS;i++) {
        sb_bar[i].sb_win = XCreateSimpleWindow(dis, root, i*sb_width, y,
                                            sb_width-bdw,sb_height,bdw,theme[3].color,theme[0].color);

        XSelectInput(dis, sb_bar[i].sb_win, ButtonPressMask|EnterWindowMask);
        XMapWindow(dis, sb_bar[i].sb_win);
    }
    sb_area = XCreateSimpleWindow(dis, root, sb_desks, y,
             sw-(sb_desks+bdw),sb_height,bdw,theme[3].color,theme[1].color);

    XMapWindow(dis, sb_area);
    status_text("");
    update_bar();
}

void toggle_bar() {
    int i;

    if(STATUS_BAR == 0) {
        if(show_bar == 1) {
            show_bar = 0;
            sh -= sb_height;
            for(i=0;i<DESKTOPS;i++) {
                XMapWindow(dis, sb_bar[i].sb_win);
                XMapWindow(dis, sb_area);
            }
        } else {
            show_bar = 1;
            sh += sb_height;
            for(i=0;i<DESKTOPS;i++) {
                XUnmapWindow(dis,sb_bar[i].sb_win);
                XUnmapWindow(dis, sb_area);
            }
        }

        tile();
        update_current();
        update_bar();
    }
}

void getwindowname() {
    char *win_name;

    if(head != NULL) {
        XFetchName(dis, current->win, &win_name);
        status_text(win_name);
        XFree(win_name);
    }
}

void status_text(const char *sb_text) {
    int text_length, text_start;

    if(sb_text == NULL) sb_text = "snapwm";
    if(head == NULL) sb_text = "snapwm";
    if(strlen(sb_text) >= 35)
        text_length = 35;
    else
        text_length = strlen(sb_text);
    text_start = 10+(XTextWidth(fontbar, theme[mode].modename, strlen(theme[mode].modename)))+(XTextWidth(fontbar, " ", 35))-(XTextWidth(fontbar, sb_text, text_length));

    XClearArea(dis, sb_area,0,0,XTextWidth(fontbar, " ", (strlen(theme[mode].modename)+40)), sb_height, False);
    XDrawString(dis, sb_area, theme[0].gc, 5, fontbar->ascent+1, theme[mode].modename, strlen(theme[mode].modename));
    XDrawString(dis, sb_area, theme[0].gc, text_start, fontbar->ascent+1, sb_text, text_length);
}

void update_bar() {
    int i;
    char busylabel[20];

    for(i=0;i<DESKTOPS;i++)
        if(i != current_desktop) {
            if(desktops[i].head != NULL) {
                strcpy(busylabel, "*"); strcat(busylabel, sb_bar[i].label);
                XSetWindowBackground(dis, sb_bar[i].sb_win, theme[2].color);
                XClearWindow(dis, sb_bar[i].sb_win);
                XDrawString(dis, sb_bar[i].sb_win, theme[1].gc, (sb_width-XTextWidth(fontbar, busylabel,strlen(busylabel)))/2, fontbar->ascent+1, busylabel, strlen(busylabel));
            } else {
                XSetWindowBackground(dis, sb_bar[i].sb_win, theme[1].color);
                XClearWindow(dis, sb_bar[i].sb_win);
                XDrawString(dis, sb_bar[i].sb_win, theme[1].gc, (sb_width-sb_bar[i].width)/2, fontbar->ascent+1, sb_bar[i].label, strlen(sb_bar[i].label));
            }
        } else {
            XSetWindowBackground(dis, sb_bar[i].sb_win, theme[0].color);
            XClearWindow(dis, sb_bar[i].sb_win);
            XDrawString(dis, sb_bar[i].sb_win, theme[1].gc, (sb_width-sb_bar[i].width)/2, fontbar->ascent+1, sb_bar[i].label, strlen(sb_bar[i].label));
        }
}

void update_output(int messg) {
    int text_length, text_start, i, j=2, k=0;
    char output[256];
    char *win_name;

    if(!(XFetchName(dis, root, &win_name))) {
        strcpy(output, "What's going on here then?");
        if(messg == 0)
            logger("\033[0;31m Failed to get status output. \n");
    } else {
        strncpy(output, win_name, strlen(win_name));
        output[strlen(win_name)] = '\0';
    }
    XFree(win_name);

    if(strlen(output) < 1) printf("\t EMPTY OUTPUT\n");
    if(strlen(output) > 255) text_length = 255;
    else text_length = strlen(output);
    for(i=0;i<text_length;i++) {
        k++;
        if(strncmp(&output[i], "&", 1) == 0)
            i += 2;
    }
    if(sw-(sb_desks+XTextWidth(fontbar, " ", (strlen(theme[mode].modename)+40))+XTextWidth(fontbar, output, k)+20) > 0)
        text_start = (XTextWidth(fontbar, " ", (strlen(theme[mode].modename)+40)))+(sw-(sb_desks+XTextWidth(fontbar, " ", (strlen(theme[mode].modename)+40))+XTextWidth(fontbar, output, k)+20));
    else
        text_start = XTextWidth(fontbar, " ", (strlen(theme[mode].modename)+40));

    XClearArea(dis, sb_area,XTextWidth(fontbar, " ", (strlen(theme[mode].modename)+40)),0,0,0, False);
    k = 0;
    for(i=0;i<text_length;i++) {
        k++;
        if(strncmp(&output[i], "&", 1) == 0) {
            j = output[i+1]-'0';
            i += 2;
        }
        XDrawString(dis, sb_area, theme[j].gc, text_start+XTextWidth(fontbar, " ", k), fontbar->ascent+1, &output[i], 1);
    }
    output[0] ='\0';
    return;
}