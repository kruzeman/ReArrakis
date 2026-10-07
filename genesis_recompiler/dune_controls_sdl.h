/* Pause menu and controller settings; bitmap text reused from RROP. */
#include <sys/stat.h>
#include <time.h>
#ifndef GENESIS_DUNE_CONTROLS_SDL_H
#define GENESIS_DUNE_CONTROLS_SDL_H
static const uint8_t dune_controls_letters[36][5]={
 {0x3e,0x51,0x49,0x45,0x3e},{0,0x42,0x7f,0x40,0},{0x42,0x61,0x51,0x49,0x46},
 {0x21,0x41,0x45,0x4b,0x31},{0x18,0x14,0x12,0x7f,0x10},{0x27,0x45,0x45,0x45,0x39},
 {0x3c,0x4a,0x49,0x49,0x30},{1,0x71,9,5,3},{0x36,0x49,0x49,0x49,0x36},
 {6,0x49,0x49,0x29,0x1e},
 {0x7e,0x11,0x11,0x11,0x7e},{0x7f,0x49,0x49,0x49,0x36},{0x3e,0x41,0x41,0x41,0x22},
 {0x7f,0x41,0x41,0x22,0x1c},{0x7f,0x49,0x49,0x49,0x41},{0x7f,9,9,9,1},
 {0x3e,0x41,0x49,0x49,0x7a},{0x7f,8,8,8,0x7f},{0,0x41,0x7f,0x41,0},
 {0x20,0x40,0x41,0x3f,1},{0x7f,8,0x14,0x22,0x41},{0x7f,0x40,0x40,0x40,0x40},
 {0x7f,2,0x0c,2,0x7f},{0x7f,4,8,0x10,0x7f},{0x3e,0x41,0x41,0x41,0x3e},
 {0x7f,9,9,9,6},{0x3e,0x41,0x51,0x21,0x5e},{0x7f,9,0x19,0x29,0x46},
 {0x46,0x49,0x49,0x49,0x31},{1,1,0x7f,1,1},{0x3f,0x40,0x40,0x40,0x3f},
 {0x1f,0x20,0x40,0x20,0x1f},{0x3f,0x40,0x38,0x40,0x3f},{0x63,0x14,8,0x14,0x63},
 {7,8,0x70,8,7},{0x61,0x51,0x49,0x45,0x43}
};
static void dune_controls_text(SDL_Renderer *r,int x,int y,int scale,const char *text,size_t max) {
    for(size_t n=0;text[n] && n<max;++n,x+=6*scale) {
        unsigned ch=(unsigned char)text[n];if(ch>='a' && ch<='z')ch-=32;
        const uint8_t *glyph=NULL;uint8_t punctuation[5]={0};
        if(ch>='0' && ch<='9')glyph=dune_controls_letters[ch-'0'];
        else if(ch>='A' && ch<='Z')glyph=dune_controls_letters[ch-'A'+10];
        else {
            if(ch=='-')memset(punctuation,8,5);
            if(ch==':')punctuation[2]=0x24;
            if(ch=='<'){punctuation[1]=8;punctuation[2]=20;punctuation[3]=34;}
            if(ch=='>'){punctuation[1]=34;punctuation[2]=20;punctuation[3]=8;}
            if(ch=='('){punctuation[2]=62;punctuation[3]=65;}
            if(ch==')'){punctuation[1]=65;punctuation[2]=62;}
            if(ch=='?'){punctuation[0]=2;punctuation[1]=1;punctuation[2]=81;punctuation[3]=9;punctuation[4]=6;}
            if(ch=='.')punctuation[2]=0x40;
            if(ch=='/') {punctuation[0]=0x20;punctuation[1]=0x10;punctuation[2]=8;punctuation[3]=4;punctuation[4]=2;}
            glyph=punctuation;
        }
        for(int col=0;col<5;++col)for(int row=0;row<7;++row)if(glyph[col]&(1<<row)) {
            SDL_Rect dot={x+col*scale,y+row*scale,scale,scale};SDL_RenderFillRect(r,&dot);
        }
    }
}
/* Diagram geometry is rasterized in drawable pixels, not enlarged pixel blocks. */
static void dune_controls_oval(SDL_Renderer *r,int x,int y,int rx,int ry,int scale) {
    rx*=scale;ry*=scale;
    for(int dy=-ry;dy<=ry;dy++) {
        int dx=rx;
        while(dx>0 && (double)dx*dx*ry*ry+(double)dy*dy*rx*rx>(double)rx*rx*ry*ry)--dx;
        SDL_RenderDrawLine(r,x-dx,y+dy,x+dx,y+dy);
    }
}
/* Front-view reference: https://www.gamerlifestore.com/products/sega-genesis-3-button-controller-original
   Hand-traced shell, action-button recess and Start; photo is not distributed. */
static void dune_controls_shape(SDL_Renderer *r,const SDL_Point *p,int n,int x,int y,int scale,SDL_Color color) {
    SDL_FPoint v[256];float top=1e9f,bottom=0;
    /* Smooth the traced outline between its measured landmarks. */
    for(int i=0;i<n;i++)for(int j=0;j<4;j++) {
        SDL_Point a=p[(i+n-1)%n],b=p[i],c=p[(i+1)%n],d=p[(i+2)%n];
        float t=j/4.f,t2=t*t,t3=t2*t;
        float px=.5f*(2*b.x+(-a.x+c.x)*t+(2*a.x-5*b.x+4*c.x-d.x)*t2+(-a.x+3*b.x-3*c.x+d.x)*t3);
        float py=.5f*(2*b.y+(-a.y+c.y)*t+(2*a.y-5*b.y+4*c.y-d.y)*t2+(-a.y+3*b.y-3*c.y+d.y)*t3);
        v[i*4+j]=(SDL_FPoint){x+px*.09f*scale,y+py*.09f*scale};
        if(v[i*4+j].y<top)top=v[i*4+j].y;if(v[i*4+j].y>bottom)bottom=v[i*4+j].y;
    }
    n*=4;
    SDL_SetRenderDrawColor(r,color.r,color.g,color.b,color.a);
    /* Scanline pairs preserve the concave grip cutout. */
    for(int row=(int)top;row<bottom;row++) {
        float cross[256],scan=row+.5f;int count=0;
        for(int i=0,j=n-1;i<n;j=i++) {
            if((v[i].y>scan)==(v[j].y>scan))continue;
            float at=v[i].x+(scan-v[i].y)*(v[j].x-v[i].x)/(v[j].y-v[i].y);
            int k=count++;while(k && cross[k-1]>at){cross[k]=cross[k-1];--k;}cross[k]=at;
        }
        for(int i=0;i+1<count;i+=2)SDL_RenderDrawLine(r,(int)(cross[i]+.5f),row,(int)cross[i+1],row);
    }
}
static void dune_controls_pad(SDLHost *h,int x,int y,int scale) {
    SDL_Renderer *r=h->renderer;x+=8*scale;y+=54*scale;
    static const SDL_Point shell[]={{12,537},{21,441},{51,351},{96,270},{168,204},{258,150},{366,102},{483,60},{615,30},{750,12},{879,0},{999,-3},{1137,9},{1269,39},{1401,81},{1530,132},{1641,189},{1746,255},{1833,333},{1896,420},{1935,519},{1950,621},{1938,729},{1905,837},{1854,933},{1788,1014},{1716,1083},{1644,1128},{1575,1143},{1512,1131},{1461,1092},{1425,1038},{1398,972},{1356,912},{1302,870},{1230,840},{1146,819},{1062,807},{969,804},{879,813},{798,834},{720,867},{657,909},{603,966},{570,1035},{537,1095},{486,1140},{423,1158},{360,1152},{294,1119},{222,1062},{159,990},{105,903},{63,810},{33,711},{15,621}};
    static const SDL_Point bezel[]={{1179,600},{1194,534},{1233,471},{1296,420},{1380,375},{1470,333},{1563,300},{1659,276},{1734,279},{1797,309},{1845,363},{1863,432},{1851,510},{1809,570},{1746,618},{1665,657},{1575,699},{1482,744},{1398,780},{1323,789},{1260,768},{1209,723},{1185,663}};
    static const SDL_Point start[]={{1311,246},{1329,234},{1443,183},{1467,189},{1482,210},{1476,234},{1362,294},{1335,300},{1317,285}};

    dune_controls_shape(r,shell,sizeof shell/sizeof *shell,x,y+2*scale,scale,(SDL_Color){7,9,12,255});
    dune_controls_shape(r,shell,sizeof shell/sizeof *shell,x,y,scale,(SDL_Color){110,119,133,255});
    dune_controls_shape(r,shell,sizeof shell/sizeof *shell,x,y+1,scale,(SDL_Color){39,43,51,255});
    dune_controls_shape(r,bezel,sizeof bezel/sizeof *bezel,x,y,scale,(SDL_Color){9,11,15,255});
    int cx=x+42*scale,cy=y+47*scale;
    SDL_SetRenderDrawColor(r,80,87,100,255);dune_controls_oval(r,cx,cy,27,27,scale);
    SDL_SetRenderDrawColor(r,12,15,21,255);dune_controls_oval(r,cx,cy,26,26,scale);
    SDL_SetRenderDrawColor(r,65,72,84,255);dune_controls_oval(r,cx,cy,19,19,scale);
    SDL_SetRenderDrawColor(r,26,30,38,255);dune_controls_oval(r,cx,cy,18,18,scale);
    SDL_SetRenderDrawColor(r,77,84,96,255);
    SDL_Rect crossx={cx-16*scale,cy-5*scale,33*scale,11*scale};SDL_RenderFillRect(r,&crossx);
    SDL_Rect crossy={cx-5*scale,cy-16*scale,11*scale,33*scale};SDL_RenderFillRect(r,&crossy);
    SDL_SetRenderDrawColor(r,27,31,38,255);dune_controls_oval(r,cx,cy,3,3,scale);
    SDL_SetRenderDrawColor(r,223,226,232,255);
    for(int row=0;row<4*scale;row++) {
        int span=row/2;
        SDL_RenderDrawLine(r,cx-span,cy-24*scale+row,cx+span,cy-24*scale+row);
        SDL_RenderDrawLine(r,cx-span,cy+24*scale-row,cx+span,cy+24*scale-row);
        SDL_RenderDrawLine(r,cx-24*scale+row,cy-span,cx-24*scale+row,cy+span);
        SDL_RenderDrawLine(r,cx+24*scale-row,cy-span,cx+24*scale-row,cy+span);
    }
    int small=(scale*2+1)/3;if(small<1)small=1;
    dune_controls_text(r,x+91*scale-12*small,y+23*scale,small,"SEGA",4);
    dune_controls_text(r,x+91*scale-21*small,y+29*scale,small,"GENESIS",7);
    int active=h->controls.remap==4;
    dune_controls_shape(r,start,sizeof start/sizeof *start,x,y+scale,scale,(SDL_Color){6,8,12,255});
    dune_controls_shape(r,start,sizeof start/sizeof *start,x,y,scale,active?(SDL_Color){240,190,80,255}:(SDL_Color){194,200,209,255});
    SDL_SetRenderDrawColor(r,223,226,232,255);
    dune_controls_text(r,x+126*scale-14*small,y+13*scale,small,"START",5);
    const int bx[3]={119,138,155},by[3]={56,46,39};const char *label[3]={"A","B","C"};
    for(int i=0;i<3;i++) {
        int px=x+bx[i]*scale,py=y+by[i]*scale;active=h->controls.remap==i+1;
        SDL_SetRenderDrawColor(r,4,6,10,255);dune_controls_oval(r,px,py+2*scale,9,9,scale);
        SDL_SetRenderDrawColor(r,active?240:107,active?190:116,active?80:130,255);dune_controls_oval(r,px,py,9,9,scale);
        SDL_SetRenderDrawColor(r,active?240:42,active?190:48,active?80:60,255);dune_controls_oval(r,px,py,8,8,scale);
        SDL_SetRenderDrawColor(r,active?20:164,active?24:173,active?30:189,255);
        dune_controls_text(r,px-2*small,py-3*small,small,label[i],1);
        SDL_SetRenderDrawColor(r,225,229,237,255);
        dune_controls_text(r,px-2*small,py-15*scale,small,label[i],1);
    }
}
static int dune_controls_read_file(SDLPadInput *p,const char *path) {
    FILE *f=fopen(path,"r");if(!f)return 0;
    SDLPadInput next=*p;sdl_pad_defaults(&next);
    unsigned version=0,b[8]={0};char extra,guid[33]={0};
    int valid=fscanf(f,"ReArrakis controls %u",&version)==1 && version>=1 && version<=4;
    if(valid)valid=fscanf(f," %d %d %d",&next.enabled,&next.layout,&next.custom)==3;
    for(int i=0;valid && i<(version==4?8:4);i++)valid=fscanf(f," %u",&b[i])==1 && b[i]<=255;
    for(int i=0;i<8;i++)next.binding[i]=(uint8_t)(i>=4 && version<4?SDL_CONTROLLER_BUTTON_DPAD_UP+i-4:b[i]);
    if(valid && version==3)valid=fscanf(f," %d",&next.cursor_stick)==1;
    if(valid && version==4){
        valid=fscanf(f," %d %d %d %d %d %d %d %d %d",&next.cursor_stick,&next.digital,&next.mouse,&next.axis_x,&next.axis_y,&next.invert_x,&next.invert_y,&next.deadzone,&next.speed)==9;
        for(int i=0;valid && i<SDL_CONTROLLER_AXIS_MAX;i++)valid=fscanf(f," %d",&next.center[i])==1 && next.center[i]>=-32768 && next.center[i]<=32767;
    }else if(next.cursor_stick==1){next.axis_x=2;next.axis_y=3;}
    if(valid && version>=2){
        valid=fscanf(f," %32s",guid)==1 && strlen(guid)==32;
        for(int i=0;valid && i<32;i++)valid=(guid[i]>='0' && guid[i]<='9') || (guid[i]>='a' && guid[i]<='f');
    }
    if(fscanf(f," %c",&extra)==1)valid=0;fclose(f);
    if(!valid || next.enabled<0 || next.enabled>1 || next.layout<0 || next.layout>1 || next.custom<0 || next.custom>1 ||
       next.cursor_stick<0 || next.cursor_stick>3 || next.digital<0 || next.digital>2 || next.mouse<0 || next.mouse>1 ||
       next.axis_x<0 || next.axis_x>=SDL_CONTROLLER_AXIS_MAX || next.axis_y<0 || next.axis_y>=SDL_CONTROLLER_AXIS_MAX ||
       next.invert_x<0 || next.invert_x>1 || next.invert_y<0 || next.invert_y>1 || next.deadzone<5 || next.deadzone>60 || next.speed<25 || next.speed>300 ||
       !sdl_pad_bindings_valid(next.binding))return 0;
    if(!strcmp(guid,"00000000000000000000000000000000"))guid[0]=0;
    memcpy(next.preferred,guid,33);*p=next;return 1;
}
static void dune_controls_read(SDLHost *h) {dune_controls_read_file(&h->input,"gamepad.cfg");}
static int dune_controls_save_file(SDLPadInput *p,const char *path) {
    char temporary[128];snprintf(temporary,sizeof temporary,"%s.tmp",path);
    FILE *f=fopen(temporary,"w");if(!f)return 0;
    int ok=fprintf(f,"ReArrakis controls 4\n%d %d %d\n",p->enabled,p->layout,p->custom)>0;
    for(int i=0;i<8;i++)if(fprintf(f,"%u ",sdl_pad_binding(p,i))<0)ok=0;
    if(fprintf(f,"\n%d %d %d %d %d %d %d %d %d\n",p->cursor_stick,p->digital,p->mouse,p->axis_x,p->axis_y,p->invert_x,p->invert_y,p->deadzone,p->speed)<0)ok=0;
    for(int i=0;i<SDL_CONTROLLER_AXIS_MAX;i++)if(fprintf(f,"%d ",p->center[i])<0)ok=0;
    if(fprintf(f,"\n%s\n",p->preferred[0]?p->preferred:"00000000000000000000000000000000")<0)ok=0;
    if(fclose(f))ok=0;
    if(ok)ok=host_file_replace(temporary,path);if(!ok)remove(temporary);return ok;
}
static void dune_controls_profile_path(SDLPadInput *p,char *path,size_t size) {
    char guid[33];SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(SDL_GameControllerGetJoystick(p->controller)),guid,sizeof guid);
    snprintf(path,size,"gamepad-%s.cfg",guid);
}
static void dune_controls_write(SDLHost *h) {
    int ok=1;
    if(h->input.controller){char path[80];dune_controls_profile_path(&h->input,path,sizeof path);ok=dune_controls_save_file(&h->input,path);}
    if(!dune_controls_save_file(&h->input,"gamepad.cfg"))ok=0;
    snprintf(h->controls.message,sizeof h->controls.message,ok?"Profile saved":"Could not save controller profile");
}
static void dune_controls_profile(SDLHost *h) {
    if(h->controls.profile_instance==h->input.instance)return;
    int initial=h->controls.profile_instance==-2;
    if(!h->input.controller){if(!initial)h->controls.profile_instance=-1;return;}
    h->controls.profile_instance=h->input.instance;h->controls.remap=0;
    char preferred[33],path[80];memcpy(preferred,h->input.preferred,33);
    dune_controls_profile_path(&h->input,path,sizeof path);
    if(!initial)sdl_pad_defaults(&h->input);dune_controls_read_file(&h->input,path);memcpy(h->input.preferred,preferred,33);
    h->input.armed=0;sdl_pad_clear(&h->input);
}
static void dune_controls_toggle(SDLHost *h,CPU *c) {
    if(h->controls.menu){h->controls.menu=0;h->controls.remap=0;h->paused=h->controls.was_paused;}
    else{h->controls.menu=1;h->controls.page=3;h->controls.selected=0;h->controls.was_paused=h->paused;h->paused=1;h->controls.message[0]=0;}
    sdl_pad_clear(&h->input);c->pad_buttons[0]=0;h->fast_forward=0;
#ifdef GENESIS_DUNE_MOUSE
    h->stick_cursor_active=0;h->stick_cursor_blocked=1;
    dune_mouse_reset(&h->dune_mouse);h->dune_view.pointer_valid=0;
#endif
    sdl_host_rebase(h,c);h->last_frame=UINT64_MAX;
}
static uint64_t dune_controls_held(SDLPadInput *p) {
    uint64_t held=0;
    for(unsigned i=0;i<SDL_PAD_BIND_COUNT;i++)if(sdl_pad_down(p,i))held|=UINT64_C(1)<<i;
    return held;
}
static void dune_controls_assign(SDLHost *h) {
    if(!h->input.controller || !SDL_GameControllerGetAttached(h->input.controller)){
        snprintf(h->controls.message,sizeof h->controls.message,"Connect and select a gamepad first");return;
    }
    h->controls.remap=h->controls.assign_one?h->controls.selected+1:1;
    h->controls.held=dune_controls_held(&h->input);h->controls.message[0]=0;
    for(int i=0;i<8;i++)h->controls.pending[i]=(uint8_t)sdl_pad_binding(&h->input,i);
    sdl_pad_clear(&h->input);
}
/* Capture before navigation: remapping B or the D-pad must not activate menus. */
static int dune_controls_capture(SDLHost *h,const SDL_Event *e) {
    DuneControls *s=&h->controls;if(!s->menu || !s->remap)return 0;
    if(((e->type==SDL_CONTROLLERDEVICEREMOVED || e->type==SDL_CONTROLLERDEVICEREMAPPED) && e->cdevice.which==h->input.instance) ||
       (e->type==SDL_WINDOWEVENT && e->window.event==SDL_WINDOWEVENT_FOCUS_LOST)){
        s->remap=0;snprintf(s->message,sizeof s->message,"Assignment cancelled");return 0;
    }
    if(e->type==SDL_KEYDOWN && !e->key.repeat && (e->key.keysym.sym==SDLK_ESCAPE || e->key.keysym.sym==SDLK_F10 || e->key.keysym.sym==SDLK_F1)){
        s->remap=0;s->message[0]=0;return 1;
    }
    int button=e->type==SDL_CONTROLLERBUTTONDOWN || e->type==SDL_CONTROLLERBUTTONUP;
    if(button || e->type==SDL_CONTROLLERAXISMOTION){
        if((button?e->cbutton.which:e->caxis.which)!=h->input.instance || !h->input.focused)return 1;
        uint64_t now=dune_controls_held(&h->input),fresh=now&~s->held;s->held=now;
        if(!fresh)return 1;
        unsigned bind=0;while(bind<SDL_PAD_BIND_COUNT && !(fresh&(UINT64_C(1)<<bind)))++bind;
        if(!sdl_pad_bindable(bind)){snprintf(s->message,sizeof s->message,"Unsupported control - ESC cancels");return 1;}
        unsigned step=s->remap-1;
        if(s->assign_one){
            /* Swap a duplicate rather than silently bind two game actions. */
            for(unsigned i=0;i<8;i++)if(i!=step && s->pending[i]==bind)s->pending[i]=s->pending[step];
        }else for(unsigned i=0;i<step;i++)if(s->pending[i]==bind){snprintf(s->message,sizeof s->message,"Already assigned - choose another");return 1;}
        s->pending[step]=(uint8_t)bind;s->message[0]=0;
        if(s->assign_one || ++s->remap==9){memcpy(h->input.binding,s->pending,8);h->input.custom=1;s->remap=0;sdl_pad_clear(&h->input);dune_controls_write(h);}
        return 1;
    }
    return e->type==SDL_KEYDOWN || e->type==SDL_KEYUP || e->type==SDL_MOUSEBUTTONDOWN;
}
static void dune_menu_path(SDLHost *h,char *path,size_t size) {
    snprintf(path,size,"state-%02d.grs",h->controls.slot+1);
}
static int dune_menu_exists(SDLHost *h) {
    char path[40];dune_menu_path(h,path,sizeof path);FILE *f=fopen(path,"rb");
    if(!f)return 0;fclose(f);return 1;
}
static void dune_menu_state(SDLHost *h,CPU *c,int load) {
    char path[40];dune_menu_path(h,path,sizeof path);
    int ok=load?state_load(c,path):state_save(c,path);
    snprintf(h->controls.message,sizeof h->controls.message,ok?(load?"State loaded - Resume when ready":"State saved"):
        (load?"Load failed: damaged or incompatible state":"Save failed: disk error or WAV recording"));
    if(ok && load){
        h->stopped=0;h->last_frame=UINT64_MAX;h->fast_forward=0;sdl_pad_clear(&h->input);h->input.armed=0;
        if(h->audio_device)SDL_ClearQueuedAudio(h->audio_device);h->audio_running=0;
#ifdef GENESIS_DUNE_MOUSE
        dune_mouse_reset(&h->dune_mouse);h->stick_cursor_active=0;h->stick_cursor_blocked=1;
        h->dune_view.camera_valid=h->dune_view.pointer_valid=h->dune_view.cursor_valid=0;
        h->dune_view.frame=UINT64_MAX;h->dune_cpu_sample_valid=h->dune_speed_valid=0;
#endif
        sdl_host_rebase(h,c);
    }
}
static void dune_menu_back(SDLHost *h,CPU *c) {
    DuneControls *s=&h->controls;s->message[0]=0;
    if(s->page==3){dune_controls_toggle(h,c);return;}
    s->page=(s->page==1 || s->page==2)?0:3;s->selected=0;
}
static const char *dune_controls_actions[8]={"A","B","C","START","UP","DOWN","LEFT","RIGHT"};
static const char *dune_controls_axes[6]={"Left X","Left Y","Right X","Right Y","Left trigger","Right trigger"};
static int dune_controls_rows(SDLHost *h) {return h->controls.page==4?2:h->controls.page==1?11:h->controls.page==0?8:9;}
static void dune_controls_change(SDLHost *h,CPU *c,int direction) {
    DuneControls *s=&h->controls;SDLPadInput *p=&h->input;int row=s->selected;
    if(s->page==4){
        int action=s->confirm;s->page=3;s->selected=0;
        if(row==1){
            if(action==8){SDL_Event quit={0};quit.type=SDL_QUIT;SDL_PushEvent(&quit);}
            else dune_menu_state(h,c,action==3);
        }
        return;
    }
    if(s->page==3){
        if(row==0){s->was_paused=0;dune_controls_toggle(h,c);return;}
        if(row==1){s->slot=(s->slot+direction+10)%10;s->message[0]=0;return;}
        if(row==2 || row==3 || row==8){
            if(row==3 && !dune_menu_exists(h)){snprintf(s->message,sizeof s->message,"This slot is empty");return;}
            if(row==2 && !dune_menu_exists(h)){dune_menu_state(h,c,0);return;}
            s->confirm=row;s->page=4;s->selected=0;s->message[0]=0;return;
        }
        if(row==4){s->page=0;s->selected=0;s->message[0]=0;return;}
        if(row==5){Uint32 flags=SDL_GetWindowFlags(h->window)&SDL_WINDOW_FULLSCREEN_DESKTOP;
            if(SDL_SetWindowFullscreen(h->window,flags?0:SDL_WINDOW_FULLSCREEN_DESKTOP))snprintf(s->message,sizeof s->message,"Could not change fullscreen mode");return;}
#ifdef GENESIS_DUNE_MOUSE
        if(row==6){h->dune_volume+=direction*5;if(h->dune_volume<0)h->dune_volume=0;if(h->dune_volume>100)h->dune_volume=100;}
        if(row==7){c->dune_cpu_double=!c->dune_cpu_double;h->dune_cpu_sample_valid=h->dune_speed_valid=0;}
#endif
        return;
    }
    if(s->page==1){
        if(row<8){s->assign_one=1;dune_controls_assign(h);return;}
        if(row==8){s->assign_one=0;dune_controls_assign(h);return;}
        if(row==9){p->custom=p->layout=0;}
        if(row==10){s->page=s->selected=0;return;}
    }else if(s->page==2){
        if(row==0){p->axis_x=(p->axis_x+direction+6)%6;p->cursor_stick=3;}
        if(row==1){p->axis_y=(p->axis_y+direction+6)%6;p->cursor_stick=3;}
        if(row==2)p->invert_x=!p->invert_x;
        if(row==3)p->invert_y=!p->invert_y;
        if(row==4){p->deadzone+=direction*5;if(p->deadzone<5)p->deadzone=5;if(p->deadzone>60)p->deadzone=60;}
        if(row==5){p->speed+=direction*25;if(p->speed<25)p->speed=25;if(p->speed>300)p->speed=300;}
        if(row==6){
            if(!p->controller){snprintf(s->message,sizeof s->message,"No controller selected");return;}
            for(int i=0;i<SDL_CONTROLLER_AXIS_MAX;i++)p->center[i]=SDL_GameControllerGetAxis(p->controller,(SDL_GameControllerAxis)i);
        }
        if(row==7){memset(p->center,0,sizeof p->center);p->deadzone=30;p->speed=100;p->invert_x=p->invert_y=0;p->axis_x=0;p->axis_y=1;p->cursor_stick=0;}
        if(row==8){s->page=s->selected=0;return;}
    }else{
        if(row==0){dune_controls_write(h);if(s->profile_instance==-2)s->profile_instance=-1;sdl_pad_cycle(p,direction);dune_controls_profile(h);}
        if(row==1)p->enabled=!p->enabled;
        if(row==2){p->cursor_stick=(p->cursor_stick+direction+3)%3;if(p->cursor_stick<2){p->axis_x=p->cursor_stick*2;p->axis_y=p->axis_x+1;}}
        if(row==3)p->digital=(p->digital+direction+3)%3;
        if(row==4)p->mouse=!p->mouse;
        if(row==5){s->page=1;s->selected=0;return;}
        if(row==6){s->page=2;s->selected=0;return;}
        if(row==7){dune_menu_back(h,c);return;}
    }
    if(p->cursor_stick!=2 && p->digital){
        int a=p->digital==1?0:2;
        if(p->axis_x==a || p->axis_x==a+1 || p->axis_y==a || p->axis_y==a+1){
            if(s->page==0 && row==3)p->cursor_stick=2;else p->digital=0;
        }
    }
    p->armed=0;sdl_pad_clear(p);c->pad_buttons[0]=0;
#ifdef GENESIS_DUNE_MOUSE
    h->stick_cursor_active=0;h->stick_cursor_blocked=1;h->dune_view.pointer_valid=0;dune_mouse_reset(&h->dune_mouse);
#endif
    dune_controls_write(h);
}
static int dune_controls_event(SDLHost *h,CPU *c,const SDL_Event *e) {
    DuneControls *s=&h->controls;
    if(e->type==SDL_KEYDOWN && !e->key.repeat){
        SDL_Keycode key=e->key.keysym.sym;
        if(key==SDLK_F1){if(s->menu && s->page==0)dune_controls_toggle(h,c);else{if(!s->menu)dune_controls_toggle(h,c);s->page=0;s->selected=0;}return 1;}
        if(key==SDLK_F10 || (!s->menu && key==SDLK_ESCAPE)){dune_controls_toggle(h,c);return 1;}
    }
    if(!s->menu)return 0;
    if(e->type==SDL_QUIT || e->type==SDL_WINDOWEVENT)return 0;
    if(e->type==SDL_KEYDOWN && !e->key.repeat){
        SDL_Keycode key=e->key.keysym.sym;int rows=dune_controls_rows(h);
        if(key==SDLK_ESCAPE){dune_menu_back(h,c);return 1;}
        if(key==SDLK_UP)s->selected=(s->selected+rows-1)%rows;
        if(key==SDLK_DOWN)s->selected=(s->selected+1)%rows;
        if(s->page==1 && s->selected<8 && (key==SDLK_DELETE || key==SDLK_BACKSPACE)){
            for(int i=0;i<8;i++)h->input.binding[i]=(uint8_t)sdl_pad_binding(&h->input,i);
            h->input.custom=1;h->input.binding[s->selected]=SDL_PAD_UNBOUND;dune_controls_write(h);
        }
        if(key==SDLK_RETURN || ((key==SDLK_LEFT || key==SDLK_RIGHT) && s->page!=4 &&
            (s->page!=3 || s->selected==1 || s->selected==6 || s->selected==7)))dune_controls_change(h,c,key==SDLK_LEFT?-1:1);
    }
    if(e->type==SDL_MOUSEBUTTONDOWN && e->button.button==SDL_BUTTON_LEFT){
        int ww,wh,w,hg;SDL_GetWindowSize(h->window,&ww,&wh);SDL_GetRendererOutputSize(h->renderer,&w,&hg);
        if(ww<=0 || wh<=0)return 1;
        int scale=w/320;if(hg/224<scale)scale=hg/224;if(scale<1)scale=1;
        int x=(e->button.x*w/ww-(w-320*scale)/2)/scale,y=(e->button.y*hg/wh-(hg-224*scale)/2)/scale;
        int left=s->page?12:200,step=s->page?12:13;
        if(x>=left && x<308 && y>=48 && y<48+dune_controls_rows(h)*step){s->selected=(y-48)/step;dune_controls_change(h,c,x<left+18?-1:1);}
    }
    return 1;
}
static void dune_controls_draw(SDLHost *h,const CPU *c) {
    (void)c;int w,hg;SDL_GetRendererOutputSize(h->renderer,&w,&hg);
    int scale=w/320;if(hg/224<scale)scale=hg/224;if(scale<1)scale=1;
    int x=(w-320*scale)/2,y=(hg-224*scale)/2;
    SDL_RenderSetLogicalSize(h->renderer,0,0);SDL_RenderSetScale(h->renderer,1,1);SDL_RenderSetViewport(h->renderer,NULL);
    SDL_SetRenderDrawColor(h->renderer,20,24,30,255);SDL_RenderClear(h->renderer);
    SDL_SetRenderDrawColor(h->renderer,240,214,140,255);
    DuneControls *s=&h->controls;SDLPadInput *p=&h->input;
    dune_controls_text(h->renderer,x+12*scale,y+12*scale,scale,s->page==1?"BUTTONS AND D-PAD":s->page==2?"ANALOG AXES AND CALIBRATION":s->page==3?"REARRAKIS - PAUSED":s->page==4?"PLEASE CONFIRM":"CONTROLLER SETTINGS",48);
    const char *name=p->controller?SDL_GameControllerName(p->controller):"Selected pad disconnected - choose Pad";
    if(s->page==3)name="Progress stays paused while this menu is open";
    if(s->page==4)name=s->confirm==2?"Overwrite this saved state?":s->confirm==3?"Load state and replace current progress?":"Quit game? Unsaved progress will be lost";
    dune_controls_text(h->renderer,x+12*scale,y+30*scale,scale,name?name:"Controller",48);
    if(!s->page)dune_controls_pad(h,x,y,scale);
    SDL_SetRenderDrawColor(h->renderer,240,214,140,255);
    int count=0,chosen=0;
    for(int i=0;i<SDL_NumJoysticks();i++)if(SDL_IsGameController(i)){++count;if(SDL_JoystickGetDeviceInstanceID(i)==p->instance)chosen=count;}
    char text[96],value[80];
    for(int i=0;i<dune_controls_rows(h);i++){
        value[0]=0;
        if(s->page==4)snprintf(value,sizeof value,"%s",i==0?"Cancel":s->confirm==2?"Overwrite state":s->confirm==3?"Load state":"Quit game");
        else if(s->page==3){
            if(i==0)snprintf(value,sizeof value,"Resume game");
            if(i==1)snprintf(value,sizeof value,"State slot: < %02d >  (%s)",s->slot+1,dune_menu_exists(h)?"Saved":"Empty");
            if(i==2)snprintf(value,sizeof value,"Save state");
            if(i==3)snprintf(value,sizeof value,"Load state%s",dune_menu_exists(h)?"":" (empty)");
            if(i==4)snprintf(value,sizeof value,"Controller settings...");
            if(i==5)snprintf(value,sizeof value,"Fullscreen: %s",SDL_GetWindowFlags(h->window)&SDL_WINDOW_FULLSCREEN_DESKTOP?"On":"Off");
#ifdef GENESIS_DUNE_MOUSE
            if(i==6)snprintf(value,sizeof value,"Volume: < %d percent >",h->dune_volume);
            if(i==7)snprintf(value,sizeof value,"CPU speed: %s",c->dune_cpu_double?"2x (debug)":"1x (original)");
#else
            if(i==6 || i==7)snprintf(value,sizeof value,"%s: unavailable",i==6?"Volume":"CPU speed");
#endif
            if(i==8)snprintf(value,sizeof value,"Quit game...");
        }else if(s->page==1){
            if(i<8)snprintf(value,sizeof value,"%s: %s",dune_controls_actions[i],s->remap==i+1?"PRESS CONTROL...":sdl_pad_label(sdl_pad_binding(p,i)));
            else snprintf(value,sizeof value,"%s",i==8?"Assign all eight":i==9?"Reset buttons":"Back");
        }else if(s->page==2){
            if(i==0 || i==1)snprintf(value,sizeof value,"Cursor %c axis: %s",i?'Y':'X',dune_controls_axes[i?p->axis_y:p->axis_x]);
            if(i==2 || i==3)snprintf(value,sizeof value,"Invert %c: %s",i==2?'X':'Y',(i==2?p->invert_x:p->invert_y)?"Yes":"No");
            if(i==4)snprintf(value,sizeof value,"Deadzone: %d percent",p->deadzone);
            if(i==5)snprintf(value,sizeof value,"Speed: %d percent",p->speed);
            if(i>=6)snprintf(value,sizeof value,"%s",i==6?"Calibrate - release sticks then click":i==7?"Reset axes and calibration":"Back");
        }else{
            if(i==0)snprintf(value,sizeof value,"Pad: %d/%d",chosen,count);
            if(i==1)snprintf(value,sizeof value,"Input: %s",p->enabled?"On":"Off");
            if(i==2)snprintf(value,sizeof value,"Cursor: %s",p->cursor_stick==0?"Left":p->cursor_stick==1?"Right":p->cursor_stick==2?"Off":"Custom");
            if(i==3)snprintf(value,sizeof value,"Digital: %s",p->digital==0?"Off":p->digital==1?"Left":"Right");
            if(i==4)snprintf(value,sizeof value,"Mouse: %s",p->mouse?"On":"Off");
            if(i>=5)snprintf(value,sizeof value,"%s",i==5?"Buttons / D-pad":i==6?"Axes / Deadzone":"Back to menu");
        }
        if(s->selected==i){
            SDL_Rect focus={x+(s->page?10:198)*scale,y+(48+i*(s->page?12:13))*scale,(s->page?298:110)*scale,(s->page?12:13)*scale};
            SDL_SetRenderDrawColor(h->renderer,65,76,90,255);SDL_RenderFillRect(h->renderer,&focus);
        }
        SDL_SetRenderDrawColor(h->renderer,240,214,140,255);
        if(s->page==3 && i==3 && !dune_menu_exists(h))SDL_SetRenderDrawColor(h->renderer,150,150,150,255);
        snprintf(text,sizeof text,"%s %s",s->selected==i?">":" ",value);
        dune_controls_text(h->renderer,x+(s->page?12:200)*scale,y+(50+i*(s->page?12:13))*scale,scale,text,s->page?48:18);
    }
    SDL_SetRenderDrawColor(h->renderer,240,214,140,255);
    if(s->page==3){
        char path[40],stamp[64]="Empty slot";struct stat info;dune_menu_path(h,path,sizeof path);
        if(!stat(path,&info)){struct tm *when=localtime(&info.st_mtime);if(when)strftime(stamp,sizeof stamp,"Saved: %Y-%m-%d %H:%M",when);}
        dune_controls_text(h->renderer,x+12*scale,y+166*scale,scale,stamp,48);
    }
    if(s->remap)snprintf(text,sizeof text,"Assign %s - release between presses",dune_controls_actions[s->remap-1]);
    else if(s->page>=3)snprintf(text,sizeof text,"Enter / A: Select   ESC / B: Back");
    else if(s->page==2 && p->controller)snprintf(text,sizeof text,"RAW %d %d  CENTER %d %d",SDL_GameControllerGetAxis(p->controller,(SDL_GameControllerAxis)p->axis_x),SDL_GameControllerGetAxis(p->controller,(SDL_GameControllerAxis)p->axis_y),p->center[p->axis_x],p->center[p->axis_y]);
    else snprintf(text,sizeof text,p->controller && !p->armed?"Center sticks or calibrate in Axes":"Only the selected pad controls the game");
    dune_controls_text(h->renderer,x+12*scale,y+181*scale,scale,text,48);
    dune_controls_text(h->renderer,x+12*scale,y+194*scale,scale,s->page==1?"Enter: Assign  Del: Clear  ESC: Back":"Arrows / Enter / Click - ESC: Back",48);
    dune_controls_text(h->renderer,x+12*scale,y+210*scale,scale,s->message,48);
    SDL_RenderPresent(h->renderer);
}
#endif
