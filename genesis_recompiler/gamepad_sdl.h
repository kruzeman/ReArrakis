/* Host input sources stay separate from serialized console state. */
#ifndef GENESIS_GAMEPAD_SDL_H
#define GENESIS_GAMEPAD_SDL_H
typedef struct {
    SDL_GameController *controller;
    SDL_JoystickID instance;
    uint8_t keyboard,blocked;
    int focused,enabled,layout,stick_x,stick_y;
    int custom;
    uint8_t binding[8];
    char preferred[33];
    int cursor_stick; /* 0: left, 1: right, 2: off, 3: custom axes */
    int axis_x,axis_y,invert_x,invert_y,deadzone,speed,digital,mouse,armed;
    int center[SDL_CONTROLLER_AXIS_MAX];
    int await_select;
} SDLPadInput;
enum { SDL_PAD_UNBOUND=255, SDL_PAD_BIND_COUNT=SDL_CONTROLLER_BUTTON_MAX+SDL_CONTROLLER_AXIS_MAX*2 };
static const uint8_t sdl_pad_bits[8]={PAD_A,PAD_B,PAD_C,PAD_START,PAD_UP,PAD_DOWN,PAD_LEFT,PAD_RIGHT};
static void sdl_pad_defaults(SDLPadInput *p) {
    p->enabled=1;p->layout=p->custom=p->cursor_stick=0;
    p->axis_x=SDL_CONTROLLER_AXIS_LEFTX;p->axis_y=SDL_CONTROLLER_AXIS_LEFTY;
    p->invert_x=p->invert_y=p->digital=p->armed=0;
    p->deadzone=30;p->speed=100;p->mouse=1;memset(p->center,0,sizeof p->center);
}
static int sdl_pad_bindable(unsigned button) {
    return button==SDL_PAD_UNBOUND || button<SDL_PAD_BIND_COUNT;
}
static int sdl_pad_bindings_valid(const uint8_t *binding) {
    for(unsigned i=0;i<8;++i) {
        if(!sdl_pad_bindable(binding[i]))return 0;
        for(unsigned j=0;j<i;++j)if(binding[i]!=SDL_PAD_UNBOUND && binding[i]==binding[j])return 0;
    }
    return 1;
}
static unsigned sdl_pad_binding(const SDLPadInput *p,unsigned action) {
    const uint8_t presets[2][8]={
        {SDL_CONTROLLER_BUTTON_X,SDL_CONTROLLER_BUTTON_A,SDL_CONTROLLER_BUTTON_B,SDL_CONTROLLER_BUTTON_START,SDL_CONTROLLER_BUTTON_DPAD_UP,SDL_CONTROLLER_BUTTON_DPAD_DOWN,SDL_CONTROLLER_BUTTON_DPAD_LEFT,SDL_CONTROLLER_BUTTON_DPAD_RIGHT},
        {SDL_CONTROLLER_BUTTON_A,SDL_CONTROLLER_BUTTON_B,SDL_CONTROLLER_BUTTON_X,SDL_CONTROLLER_BUTTON_START,SDL_CONTROLLER_BUTTON_DPAD_UP,SDL_CONTROLLER_BUTTON_DPAD_DOWN,SDL_CONTROLLER_BUTTON_DPAD_LEFT,SDL_CONTROLLER_BUTTON_DPAD_RIGHT}
    };
    return p->custom ? p->binding[action]:presets[p->layout!=0][action];
}
static double sdl_pad_value(SDLPadInput *p,int axis) {
    if(!p->controller || !SDL_GameControllerGetAttached(p->controller))return 0;
    int value=SDL_GameControllerGetAxis(p->controller,(SDL_GameControllerAxis)axis)-p->center[axis];
    int dead=p->deadzone*32768/100,magnitude=value<0?-value:value;
    if(magnitude<=dead)return 0;
    int range=(value<0?32768+p->center[axis]:32767-p->center[axis])-dead;
    if(range<=0)return 0;
    double result=(double)(magnitude-dead)/range;if(result>1)result=1;
    return value<0?-result:result;
}
static int sdl_pad_down(SDLPadInput *p,unsigned bind) {
    if(!p->controller || bind==SDL_PAD_UNBOUND)return 0;
    if(bind<SDL_CONTROLLER_BUTTON_MAX)return SDL_GameControllerGetButton(p->controller,(SDL_GameControllerButton)bind);
    unsigned axis=(bind-SDL_CONTROLLER_BUTTON_MAX)/2,positive=(bind-SDL_CONTROLLER_BUTTON_MAX)%2;
    double value=sdl_pad_value(p,(int)axis);return positive?value>.5:value<-.5;
}
static int sdl_pad_ready(SDLPadInput *p) {
    if(!p->controller || !SDL_GameControllerGetAttached(p->controller))return p->armed=0;
    if(!p->armed) {
        if(p->cursor_stick!=2 && (sdl_pad_value(p,p->axis_x) || sdl_pad_value(p,p->axis_y)))return 0;
        if(p->digital){int a=p->digital==1?0:2;if(sdl_pad_value(p,a) || sdl_pad_value(p,a+1))return 0;}
        for(int i=0;i<8;i++)if(sdl_pad_binding(p,i)>=SDL_CONTROLLER_BUTTON_MAX && sdl_pad_down(p,sdl_pad_binding(p,i)))return 0;
        p->armed=1;
    }
    return 1;
}
static const char *sdl_pad_label(unsigned button) {
    if(button==SDL_PAD_UNBOUND)return "Unbound";
    if(button>=SDL_CONTROLLER_BUTTON_MAX){
        static const char *axes[12]={"LX-","LX+","LY-","LY+","RX-","RX+","RY-","RY+","LT-","LT+","RT-","RT+"};
        return button<SDL_PAD_BIND_COUNT?axes[button-SDL_CONTROLLER_BUTTON_MAX]:"Invalid";
    }
    switch(button) {
        case SDL_CONTROLLER_BUTTON_A:return "A";
        case SDL_CONTROLLER_BUTTON_B:return "B";
        case SDL_CONTROLLER_BUTTON_X:return "X";
        case SDL_CONTROLLER_BUTTON_Y:return "Y";
        case SDL_CONTROLLER_BUTTON_START:return "Start";
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:return "L3";
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:return "R3";
        default: {
            const char *name=SDL_GameControllerGetStringForButton((SDL_GameControllerButton)button);
            return name ? name:"Unknown";
        }
    }
}
static int sdl_pad_axis(int value,int previous) {
    if(value<=-10000)return -1;
    if(value>=10000)return 1;
    if(previous<0 && value<-8000)return -1;
    if(previous>0 && value>8000)return 1;
    return 0;
}
static uint8_t sdl_pad_physical(SDLPadInput *p) {
    if(!sdl_pad_ready(p))return 0;
    uint8_t buttons=0;
    for(unsigned i=0;i<8;i++)if(sdl_pad_down(p,sdl_pad_binding(p,i)))buttons|=sdl_pad_bits[i];
    if(p->digital) {
        int a=p->digital==1?0:2;double x=sdl_pad_value(p,a),y=sdl_pad_value(p,a+1);
        if(x<-.3)buttons|=PAD_LEFT;if(x>.3)buttons|=PAD_RIGHT;
        if(y<-.3)buttons|=PAD_UP;if(y>.3)buttons|=PAD_DOWN;
    }
    return buttons;
}
static void sdl_pad_clear(SDLPadInput *p) {
    p->keyboard=0;p->blocked=sdl_pad_physical(p);
}
static void sdl_pad_close(SDLPadInput *p) {
    if(p->controller)SDL_GameControllerClose(p->controller);
    p->controller=NULL;p->instance=-1;p->stick_x=p->stick_y=0;p->blocked=0;p->armed=0;
}
static int sdl_pad_select(SDLPadInput *p,int index) {
    SDL_GameController *controller=SDL_GameControllerOpen(index);if(!controller)return 0;
    sdl_pad_close(p);p->controller=controller;
    p->instance=SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
    SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(index),p->preferred,sizeof p->preferred);
    p->await_select=0;p->armed=0;sdl_pad_clear(p);
    fprintf(stderr,"gamepad: %s connected\n",SDL_GameControllerName(controller));return 1;
}
static void sdl_pad_connect(SDLPadInput *p) {
    if(p->controller)return;
    int first=-1;
    for(int i=0;i<SDL_NumJoysticks();++i) {
        if(!SDL_IsGameController(i))continue;if(first<0)first=i;
        char guid[33];SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(i),guid,sizeof guid);
        if(!strcmp(guid,p->preferred) && sdl_pad_select(p,i))return;
    }
    if(!p->preferred[0] && first>=0)sdl_pad_select(p,first);
}
static void sdl_pad_cycle(SDLPadInput *p,int direction) {
    int n=SDL_NumJoysticks(),current=direction>0?-1:0;
    for(int i=0;i<n;i++)if(SDL_JoystickGetDeviceInstanceID(i)==p->instance)current=i;
    for(int step=1;step<=n;step++) {
        int index=(current+direction*step+n*2)%n;
        if(SDL_IsGameController(index) && sdl_pad_select(p,index)) {
            /* ponytail: GUID remembers the model; identical pads use first match on restart. */
            SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(index),p->preferred,sizeof p->preferred);return;
        }
    }
}
static uint8_t sdl_pad_buttons(SDLPadInput *p,int suspended) {
    uint8_t physical=sdl_pad_physical(p);
    if(suspended || !p->focused) {
        p->keyboard=0;p->blocked=physical;return 0;
    }
    p->blocked&=physical;
    if(!p->enabled) {p->blocked=physical;physical=0;}
    uint8_t buttons=p->keyboard|(physical&(uint8_t)~p->blocked);
    if((buttons&(PAD_UP|PAD_DOWN))==(PAD_UP|PAD_DOWN))buttons&=(uint8_t)~(PAD_UP|PAD_DOWN);
    if((buttons&(PAD_LEFT|PAD_RIGHT))==(PAD_LEFT|PAD_RIGHT))buttons&=(uint8_t)~(PAD_LEFT|PAD_RIGHT);
    return buttons;
}
/* Translate only host-menu gestures. Native game input is polled separately. */
static int sdl_pad_event(SDLPadInput *p,SDL_Event *event,int menu) {
    if(event->type==SDL_CONTROLLERDEVICEADDED) {if(!p->await_select)sdl_pad_connect(p);return 1;}
    if(event->type==SDL_CONTROLLERDEVICEREMOVED) {
        if(event->cdevice.which==p->instance) {sdl_pad_close(p);p->await_select=1;}
        return 1;
    }
    if(event->type==SDL_CONTROLLERDEVICEREMAPPED) {if(event->cdevice.which==p->instance){p->armed=0;sdl_pad_clear(p);}return 1;}
    int button=event->type==SDL_CONTROLLERBUTTONDOWN || event->type==SDL_CONTROLLERBUTTONUP;
    int axis=event->type==SDL_CONTROLLERAXISMOTION;
    if(!button && !axis)return 0;
    SDL_JoystickID which=button ? event->cbutton.which:event->caxis.which;
    if(which!=p->instance || !p->enabled || !p->focused)return 1;
    SDL_Keycode key=0;int down=button && event->type==SDL_CONTROLLERBUTTONDOWN;
    if(button) {
        unsigned b=event->cbutton.button;
        if(menu) {
            if(b==SDL_CONTROLLER_BUTTON_DPAD_UP)key=SDLK_UP;
            if(b==SDL_CONTROLLER_BUTTON_DPAD_DOWN)key=SDLK_DOWN;
            if(b==SDL_CONTROLLER_BUTTON_DPAD_LEFT)key=SDLK_LEFT;
            if(b==SDL_CONTROLLER_BUTTON_DPAD_RIGHT)key=SDLK_RIGHT;
            if(b==SDL_CONTROLLER_BUTTON_A || b==SDL_CONTROLLER_BUTTON_X || b==SDL_CONTROLLER_BUTTON_START)key=SDLK_RETURN;
            if(b==SDL_CONTROLLER_BUTTON_B)key=SDLK_ESCAPE;
        }
        if(b==SDL_CONTROLLER_BUTTON_BACK){
            int bound=0;for(int i=0;i<8;i++)if(sdl_pad_binding(p,i)==b)bound=1;
            if(menu || !bound)key=menu?SDLK_ESCAPE:SDLK_F10;
        }
    }

    if(!key)return 1;
    SDL_zero(*event);event->type=down ? SDL_KEYDOWN:SDL_KEYUP;
    event->key.state=down ? SDL_PRESSED:SDL_RELEASED;event->key.keysym.sym=key;
    return 0;
}
#endif
