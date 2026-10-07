#ifndef GENESIS_DUNE_VIEW_SDL_H
#define GENESIS_DUNE_VIEW_SDL_H
#if defined(GENESIS_DUNE_MOUSE) && defined(GENESIS_SDL2)
#include "dune_cpu_sdl.h"
static int dune_view_map(const CPU *c){return !c->fault && dune_mouse_context(c)==1 && c->vdp.frame_width==320 && c->vdp.frame_height==224;}
static DuneLayout dune_view_host_layout(SDLHost *h,const CPU *c,int w,int height){
    DuneView *v=&h->dune_view;DuneMouse *m=&h->dune_mouse;
    int cx=dune_word(c,0xe3ec),cy=dune_word(c,0xe3ee);
    if(!v->camera_valid){v->camera_x=cx;v->camera_y=cy;v->last_camera_x=cx;v->last_camera_y=cy;v->camera_valid=1;}
    /* Internal camera movement makes the original game able to pick an
       offscreen target. It must not move the player's expanded overview. */
    if(!m->camera_hold && !v->pointer_valid){
        v->camera_x+=cx-v->last_camera_x;v->camera_y+=cy-v->last_camera_y;
    }else if(!m->count && (m->pan_x || m->pan_y)){
        uint64_t elapsed=c->vdp.rendered_frames-v->pan_frame;if(elapsed>4)elapsed=4;
        v->camera_x+=m->pan_x*(int)elapsed;v->camera_y+=m->pan_y*(int)elapsed;
        int minx=dune_word(c,0xe400),maxx=dune_word(c,0xe402),miny=dune_word(c,0xe3fc),maxy=dune_word(c,0xe3fe);
        if(v->camera_x<minx)v->camera_x=minx;if(v->camera_x>maxx)v->camera_x=maxx;
        if(v->camera_y<miny)v->camera_y=miny;if(v->camera_y>maxy)v->camera_y=maxy;
    }
    v->pan_frame=c->vdp.rendered_frames;
    v->last_camera_x=cx;v->last_camera_y=cy;
    DuneLayout p=dune_view_layout(c,w,height,v->zoom);
    p.left+=v->camera_x-cx;p.top+=v->camera_y-cy;return p;
}
static int dune_view_mouse(SDLHost *h,const CPU *c,int mx,int my,int *x,int *y,int *world){
    *world=0;*x=mx;*y=my;if(!h->renderer)return 1;
    int ww,wh,w,hg;SDL_GetWindowSize(h->window,&ww,&wh);
    if(ww<=0 || wh<=0 || SDL_GetRendererOutputSize(h->renderer,&w,&hg))return 0;
    if(mx<0 || my<0 || mx>=ww || my>=wh)return 0;
    double px=(double)mx*w/ww,py=(double)my*hg/wh;
    DuneLayout p=dune_view_map(c)?dune_view_host_layout(h,c,w,hg):dune_view_layout(c,w,hg,h->dune_view.zoom);
    if(!p.ui_scale)return 0;
    if(!dune_view_map(c)){
        *x=(int)((px-p.ui_x)/p.ui_scale);*y=(int)((py-p.ui_y)/p.ui_scale);
        return px>=p.ui_x && py>=p.ui_y && *x<(int)c->vdp.frame_width && *y<(int)c->vdp.frame_height;
    }
    double ux=(px-(w-320*p.ui_scale))/p.ui_scale,uy=py/p.ui_scale;
    double bottom_y=(py-(hg-224*p.ui_scale))/p.ui_scale;
    int top_hit=0;
    int bottom_hit=ux>=240 && ux<304 && bottom_y>=144 && bottom_y<208;
    if(h->dune_view.ui && ux>=0 && ux<320){
        if(uy>=0 && uy<144 && h->dune_view.ui[((int)uy*320+(int)ux)*4+3])top_hit=1;
        if(bottom_y>=144 && bottom_y<224 && h->dune_view.ui[((int)bottom_y*320+(int)ux)*4+3])bottom_hit=1;
    }
    if(top_hit || bottom_hit){*x=(int)ux;*y=(int)(bottom_hit?bottom_y:uy);return 1;}
    *world=1;*x=p.left+(int)(px/p.world_scale);*y=p.top+(int)(py/p.world_scale);
    return *x>=0 && *y>=0 && *x<2048 && *y<2048;
}
static void dune_view_pointer(SDLHost *h,const CPU *c){
    DuneView *v=&h->dune_view;DuneMouse *m=&h->dune_mouse;
    if(!v->pointer_valid)return;
    int x,y,world;
    if(!dune_view_mouse(h,c,v->pointer_x,v->pointer_y,&x,&y,&world)){
        if(dune_view_map(c)){m->camera_hold=1;m->pan_x=m->pan_y=0;}
        return;
    }
    if(world)dune_mouse_world_point(m,c,x,y);else dune_mouse_point(m,c,x,y);
    m->pan_x=m->pan_y=0;
    if(world && h->window){
        int w,height;if(SDL_GetRendererOutputSize(h->renderer,&w,&height))return;
        DuneLayout p=dune_view_host_layout(h,c,w,height);
        /* Match the visible cursor: a snapped 32px cell or the free square
           spanning -13..12 around its anchor. Compare in drawable pixels,
           so zoom and Retina scaling affect scrolling just like rendering. */
        int grid=dune_long(c,0xe002)==0x6092;
        int left=grid?(x&~31):x-13,top=grid?(y&~31):y-13;
        int size=grid?32:26;
        m->pan_x=(left-p.left)*p.world_scale<=0?-3:
                 (left+size-p.left)*p.world_scale>=w?3:0;
        m->pan_y=(top-p.top)*p.world_scale<=0?-3:
                 (top+size-p.top)*p.world_scale>=height?3:0;
    }
}
static int dune_view_event(SDLHost *h,CPU *c,const SDL_Event *e){
    if(!h->dune_view.zoom)h->dune_view.zoom=100;
    if(!h->paused && !h->stopped && dune_view_map(c) &&
       e->type==SDL_MOUSEBUTTONDOWN && e->button.button==SDL_BUTTON_LEFT){
        int x,y,world;
        if(dune_view_mouse(h,c,e->button.x,e->button.y,&x,&y,&world) &&
           !world && x>=240 && x<304 && y>=144 && y<208){
            /* One minimap pixel represents one 32px world cell. Center the
               independent overview; do not send A/B or change unit selection. */
            DuneView *v=&h->dune_view;
            v->camera_x=(x-240)*32+16-160;
            v->camera_y=(y-144)*32+16-112;
            int minx=dune_word(c,0xe400),maxx=dune_word(c,0xe402);
            int miny=dune_word(c,0xe3fc),maxy=dune_word(c,0xe3fe);
            if(v->camera_x<minx)v->camera_x=minx;if(v->camera_x>maxx)v->camera_x=maxx;
            if(v->camera_y<miny)v->camera_y=miny;if(v->camera_y>maxy)v->camera_y=maxy;
            v->camera_valid=1;v->last_camera_x=dune_word(c,0xe3ec);v->last_camera_y=dune_word(c,0xe3ee);
            v->pan_frame=c->vdp.rendered_frames;v->frame=UINT64_MAX;
            v->pointer_valid=1;v->pointer_x=e->button.x;v->pointer_y=e->button.y;
            dune_mouse_reset(&h->dune_mouse);h->dune_mouse.camera_hold=1;
            dune_view_pointer(h,c);return 1;
        }
    }
    if(!h->paused && !h->stopped && dune_view_map(c) && e->type==SDL_MOUSEWHEEL){
        int w,height;if(SDL_GetRendererOutputSize(h->renderer,&w,&height) || w<=0 || height<=0)return 0;
        DuneLayout old=dune_view_host_layout(h,c,w,height);
        int ax,ay,world=0;
        int anchor=h->dune_view.pointer_valid && dune_view_mouse(h,c,h->dune_view.pointer_x,h->dune_view.pointer_y,&ax,&ay,&world) && world;
        int delta=e->wheel.y;if(delta>20)delta=20;if(delta<-20)delta=-20;
        if(e->wheel.direction==SDL_MOUSEWHEEL_FLIPPED)delta=-delta;
        int zoom=(int)h->dune_view.zoom+delta*10;if(zoom<50)zoom=50;if(zoom>100)zoom=100;
        h->dune_view.zoom=zoom;
        if(anchor){
            int ww,wh;SDL_GetWindowSize(h->window,&ww,&wh);
            double px=(double)h->dune_view.pointer_x*w/ww,py=(double)h->dune_view.pointer_y*height/wh;
            DuneLayout next=dune_view_host_layout(h,c,w,height);
            h->dune_view.camera_x+=(int)(old.left+px/old.world_scale-next.left-px/next.world_scale);
            h->dune_view.camera_y+=(int)(old.top+py/old.world_scale-next.top-py/next.world_scale);
        }
        dune_mouse_reset(&h->dune_mouse);dune_view_pointer(h,c);return !!delta;
    }
    if(e->type==SDL_KEYDOWN && !e->key.repeat && e->key.keysym.sym==SDLK_F4){
        c->dune_cpu_double=!c->dune_cpu_double;
        h->dune_cpu_sample_valid=0;h->dune_speed_valid=0;return 1;
    }
    if(e->type==SDL_KEYDOWN && !e->key.repeat && e->key.keysym.sym==SDLK_F3){
        h->dune_cpu_overlay=!h->dune_cpu_overlay;h->dune_cpu_sample_valid=0;h->dune_speed_valid=0;return 1;
    }
    if(e->type==SDL_KEYDOWN && !e->key.repeat && e->key.keysym.sym==SDLK_0){
        h->dune_view.zoom=100;dune_mouse_reset(&h->dune_mouse);dune_view_pointer(h,c);return 1;
    }
    if(e->type==SDL_KEYDOWN && !e->key.repeat && e->key.keysym.sym==SDLK_F11 && h->window){
        Uint32 flag=SDL_GetWindowFlags(h->window)&SDL_WINDOW_FULLSCREEN_DESKTOP;
        if(SDL_SetWindowFullscreen(h->window,flag?0:SDL_WINDOW_FULLSCREEN_DESKTOP))return 0;
        dune_mouse_reset(&h->dune_mouse);return 1;
    }
    if(e->type==SDL_WINDOWEVENT && e->window.event==SDL_WINDOWEVENT_SIZE_CHANGED)dune_mouse_reset(&h->dune_mouse);
    return 0;
}
static int dune_view_draw(SDLHost *h,CPU *c){
    VDP *v=&c->vdp;if(!h->dune_view.zoom)h->dune_view.zoom=100;
    int window_w,window_h;SDL_GetWindowSize(h->window,&window_w,&window_h);
    if(window_w!=h->dune_window_width || window_h!=h->dune_window_height){
        /* SDL's software renderer refreshes its window surface lazily. Flush
           once on resize before querying pixels and resetting the viewport. */
        if(SDL_RenderSetLogicalSize(h->renderer,0,0) || SDL_RenderSetScale(h->renderer,1,1) ||
           SDL_RenderSetViewport(h->renderer,NULL) || SDL_RenderClear(h->renderer) ||
           SDL_RenderFlush(h->renderer))return sdl_host_error(h,"Dune resize failed");
        h->dune_window_width=window_w;h->dune_window_height=window_h;
    }
    int w,hg;if(SDL_GetRendererOutputSize(h->renderer,&w,&hg))return sdl_host_error(h,"Dune output size failed");
    if(w<=0 || hg<=0 || !v->frame_width || !v->frame_height)return 1;
    if(SDL_RenderSetLogicalSize(h->renderer,0,0) || SDL_RenderSetScale(h->renderer,1,1) ||
       SDL_RenderSetViewport(h->renderer,NULL))return sdl_host_error(h,"Dune viewport failed");
    DuneLayout p=dune_view_map(c)?dune_view_host_layout(h,c,w,hg):dune_view_layout(c,w,hg,h->dune_view.zoom);
    if(SDL_RenderClear(h->renderer))return sdl_host_error(h,"Dune clear failed");
    if(!dune_view_map(c)){
        if(dune_mouse_active(&h->dune_mouse,c)>=4)h->dune_view.camera_valid=0;
        if(!h->texture || h->width!=v->frame_width || h->height!=v->frame_height){
            SDL_DestroyTexture(h->texture);h->texture=SDL_CreateTexture(h->renderer,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,v->frame_width,v->frame_height);
            if(!h->texture)return sdl_host_error(h,"Dune native texture failed");
            h->width=v->frame_width;h->height=v->frame_height;
        }
        SDL_FRect to={(float)p.ui_x,(float)p.ui_y,(float)(v->frame_width*p.ui_scale),(float)(v->frame_height*p.ui_scale)};
        if(SDL_UpdateTexture(h->texture,NULL,v->frame,v->frame_width*3) || SDL_RenderCopyF(h->renderer,h->texture,NULL,&to))return sdl_host_error(h,"Dune native draw failed");
    }else{
        DuneView *d=&h->dune_view;
        dune_view_pointer(h,c);
        int hide_cursor=h->stick_cursor_active || (d->pointer_valid && !h->dune_mouse.point_world);
        if(d->cursor_hidden!=hide_cursor)d->frame=UINT64_MAX;
        d->cursor_hidden=hide_cursor;
        if(d->cursor_valid!=h->dune_mouse.point_world || d->cursor_x!=h->dune_mouse.wx || d->cursor_y!=h->dune_mouse.wy)d->frame=UINT64_MAX;
        d->cursor_valid=h->dune_mouse.point_world;d->cursor_x=h->dune_mouse.wx;d->cursor_y=h->dune_mouse.wy;
        /* Replay is bounded and skipped on expose events for an unchanged frame/layout. */
        if(d->frame!=v->rendered_frames || d->width!=(unsigned)p.world_width || d->height!=(unsigned)p.world_height || d->left!=p.left || d->top!=p.top){
            if(!dune_view_build(d,c,&p))return sdl_host_error(h,"Dune world traversal failed");
            if(!h->dune_world || h->dune_width!=d->width || h->dune_height!=d->height){
                SDL_DestroyTexture(h->dune_world);h->dune_world=SDL_CreateTexture(h->renderer,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,d->width,d->height);
                if(!h->dune_world)return sdl_host_error(h,"Dune world texture failed");
                h->dune_width=d->width;h->dune_height=d->height;
            }
            if(!h->dune_hud){h->dune_hud=SDL_CreateTexture(h->renderer,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STREAMING,320,224);
                if(!h->dune_hud || SDL_SetTextureBlendMode(h->dune_hud,SDL_BLENDMODE_BLEND))return sdl_host_error(h,"Dune HUD texture failed");}
            if(SDL_UpdateTexture(h->dune_world,NULL,d->pixels,d->width*3) || SDL_UpdateTexture(h->dune_hud,NULL,d->ui,320*4))return sdl_host_error(h,"Dune world upload failed");
        }
        SDL_FRect to={0,0,(float)(d->width*p.world_scale),(float)(d->height*p.world_scale)};
        if(SDL_RenderCopyF(h->renderer,h->dune_world,NULL,&to))return sdl_host_error(h,"Dune world draw failed");
        SDL_Rect top={0,0,320,144},bottom={0,144,320,80};
        SDL_FRect t={(float)(w-320*p.ui_scale),0,(float)(320*p.ui_scale),(float)(144*p.ui_scale)};
        SDL_FRect b={t.x,(float)(hg-80*p.ui_scale),t.w,(float)(80*p.ui_scale)};
        if(SDL_RenderCopyF(h->renderer,h->dune_hud,&top,&t) || SDL_RenderCopyF(h->renderer,h->dune_hud,&bottom,&b))return sdl_host_error(h,"Dune HUD draw failed");
    }
    if(!dune_cpu_draw(h,c))return sdl_host_error(h,"Dune CPU overlay failed");
    h->last_frame=v->rendered_frames;SDL_RenderPresent(h->renderer);return 1;
}
#endif
#endif
