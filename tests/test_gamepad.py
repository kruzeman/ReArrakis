"""Button assignment through SDL events, physical polling and persisted settings."""
import os
import subprocess
import unittest

from genesis_recompiler.build import BuildError, sdl2_flags
from genesis_recompiler.decode import analyze
from genesis_recompiler.emit import emit
from support import CompiledTestCase, rom_with


class GamepadRemapTests(CompiledTestCase):
    @classmethod
    def setUpClass(cls):
        try:
            cls.flags,cls.libs=sdl2_flags()
        except BuildError as exc:
            raise unittest.SkipTest(str(exc))

    def check(self, body):
        source='#define GENESIS_NO_MAIN\n#define GENESIS_DUNE_MOUSE\n'+emit(analyze(rom_with('4e72 2700'),[0x200]))
        source+=r'''
#include <assert.h>
static void key(SDLHost *h,CPU *c,SDL_Keycode key) {
 SDL_Event e={0};e.type=SDL_KEYDOWN;e.key.keysym.sym=key;assert(SDL_PushEvent(&e)==1);
 assert(sdl_host_service(h,c));
}
static void button(SDLHost *h,CPU *c,SDL_Joystick *j,unsigned b,int down) {
 assert(!SDL_JoystickSetVirtualButton(j,(int)b,(Uint8)down));SDL_PumpEvents();
 assert(sdl_host_service(h,c));
}
static unsigned pixel(SDLHost *h,int x,int y) {
 Uint32 value=0;SDL_Rect rect={x,y,1,1};
 assert(!SDL_RenderReadPixels(h->renderer,&rect,SDL_PIXELFORMAT_ARGB8888,&value,sizeof value));
 return value&0xffffff;
}
static void begin(SDLHost *h,CPU *c) {
 if(!h->controls.menu)dune_controls_toggle(h,c);
 h->controls.page=1;h->controls.selected=8;
 key(h,c,SDLK_RETURN);assert(h->controls.remap==1 && !c->pad_buttons[0]);
}
int main(void) {
 CPU *c=calloc(1,sizeof *c);assert(c);c->rom=rom_data;c->rom_size=sizeof rom_data;
 SDLHost h={0};h.no_throttle=1;assert(sdl_host_open(&h));
 int device=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,SDL_CONTROLLER_AXIS_MAX,SDL_CONTROLLER_BUTTON_MAX,0);
 assert(device>=0);SDL_Joystick *j=SDL_JoystickOpen(device);assert(j);
 sdl_pad_connect(&h.input);assert(h.input.controller && h.input.instance==SDL_JoystickInstanceID(j));
 assert(sdl_host_service(&h,c));
''' + body + r'''
 SDL_JoystickClose(j);sdl_host_close(&h);free(c);return 0;
}
'''
        path=self.root/'remap.c';path.write_text(source);binary=self.root/'remap'
        result=subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-missing-field-initializers',
                               '-DGENESIS_SDL2',*self.flags,str(path),'-o',str(binary),*self.libs],capture_output=True,text=True)
        self.assertEqual(result.returncode,0,result.stderr)
        result=subprocess.run([str(binary)],cwd=self.root,env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software'),
                              capture_output=True,text=True,timeout=15)
        self.assertEqual(result.returncode,0,result.stderr)

    def test_configure_isolate_calibrate_and_persist_controllers(self):
        self.check(r'''
SDL_SetWindowSize(h.window,320,224);
key(&h,c,SDLK_ESCAPE);assert(h.controls.menu);key(&h,c,SDLK_ESCAPE);assert(!h.controls.menu);
begin(&h,c);
unsigned mapping[8]={SDL_CONTROLLER_BUTTON_Y,SDL_CONTROLLER_BUTTON_B,SDL_CONTROLLER_BUTTON_A,SDL_CONTROLLER_BUTTON_X,
 SDL_CONTROLLER_BUTTON_DPAD_UP,SDL_CONTROLLER_BUTTON_DPAD_DOWN,SDL_CONTROLLER_BUTTON_DPAD_LEFT,SDL_CONTROLLER_BUTTON_DPAD_RIGHT};
for(unsigned i=0;i<8;i++){
 button(&h,c,j,mapping[i],1);assert(!c->pad_buttons[0]);
 if(i<7)assert(h.controls.remap==(int)i+2);else assert(!h.controls.remap && h.input.custom);
 button(&h,c,j,mapping[i],0);
}
dune_controls_toggle(&h,c);
for(unsigned i=0;i<8;i++){
 button(&h,c,j,mapping[i],1);assert(c->pad_buttons[0]==sdl_pad_bits[i]);
 button(&h,c,j,mapping[i],0);assert(!c->pad_buttons[0]);
}
h.input.custom=0;dune_controls_read(&h);assert(h.input.custom);
for(unsigned i=0;i<8;i++)assert(sdl_pad_binding(&h.input,i)==mapping[i]);
/* A pre-held unrelated control must not block remapping a fresh direction. */
dune_controls_toggle(&h,c);button(&h,c,j,SDL_CONTROLLER_BUTTON_LEFTSHOULDER,1);
h.controls.page=1;h.controls.selected=4;key(&h,c,SDLK_RETURN);
button(&h,c,j,SDL_CONTROLLER_BUTTON_DPAD_UP,1);assert(!h.controls.remap);
button(&h,c,j,SDL_CONTROLLER_BUTTON_DPAD_UP,0);button(&h,c,j,SDL_CONTROLLER_BUTTON_LEFTSHOULDER,0);
dune_controls_toggle(&h,c);
/* Individual D-pad assignment may use an axis, and Delete clears it. */
dune_controls_toggle(&h,c);h.controls.page=1;h.controls.selected=4;key(&h,c,SDLK_RETURN);
assert(h.controls.remap==5);
assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_RIGHTY,-32768));SDL_PumpEvents();assert(sdl_host_service(&h,c));
assert(!h.controls.remap && sdl_pad_binding(&h.input,4)==SDL_CONTROLLER_BUTTON_MAX+6);
assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_RIGHTY,0));assert(sdl_host_service(&h,c));
dune_controls_toggle(&h,c);assert(sdl_host_service(&h,c));
assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_RIGHTY,-32768));assert(sdl_host_service(&h,c));assert(c->pad_buttons[0]==PAD_UP);
assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_RIGHTY,0));assert(sdl_host_service(&h,c));
dune_controls_toggle(&h,c);h.controls.page=1;h.controls.selected=4;key(&h,c,SDLK_DELETE);assert(sdl_pad_binding(&h.input,4)==SDL_PAD_UNBOUND);
key(&h,c,SDLK_RETURN);button(&h,c,j,SDL_CONTROLLER_BUTTON_DPAD_UP,1);button(&h,c,j,SDL_CONTROLLER_BUTTON_DPAD_UP,0);
assert(sdl_pad_binding(&h.input,4)==SDL_CONTROLLER_BUTTON_DPAD_UP);
/* Different controller model gets its own settings; nonselected events are inert. */
SDL_VirtualJoystickDesc desc={0};desc.version=SDL_VIRTUAL_JOYSTICK_DESC_VERSION;
desc.type=SDL_JOYSTICK_TYPE_GAMECONTROLLER;desc.naxes=SDL_CONTROLLER_AXIS_MAX;desc.nbuttons=SDL_CONTROLLER_BUTTON_MAX;
desc.vendor_id=0x045e;desc.product_id=0x02fd;desc.name="Second test controller";
int second=SDL_JoystickAttachVirtualEx(&desc);assert(second>=0);SDL_Joystick *other=SDL_JoystickOpen(second);assert(other);
h.controls.page=0;h.controls.selected=0;key(&h,c,SDLK_RIGHT);
assert(h.input.instance==SDL_JoystickInstanceID(other) && !h.input.custom);
h.input.deadzone=45;dune_controls_write(&h);
key(&h,c,SDLK_LEFT);assert(h.input.instance==SDL_JoystickInstanceID(j) && h.input.custom && h.input.deadzone==30);
key(&h,c,SDLK_RIGHT);assert(h.input.deadzone==45 && !h.input.custom);
dune_controls_toggle(&h,c);assert(sdl_host_service(&h,c));
button(&h,c,j,SDL_CONTROLLER_BUTTON_A,1);assert(!c->pad_buttons[0]);button(&h,c,j,SDL_CONTROLLER_BUTTON_A,0);
assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_LEFTX,-32768));assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_LEFTY,-32768));
assert(sdl_host_service(&h,c));assert(!h.dune_view.pointer_valid && !c->pad_buttons[0]);
SDL_Event e={0};e.type=SDL_CONTROLLERDEVICEREMAPPED;e.cdevice.which=SDL_JoystickInstanceID(j);
int armed=h.input.armed;assert(SDL_PushEvent(&e)==1);assert(sdl_host_service(&h,c));assert(h.input.armed==armed);
/* Unplugging never silently hands control to another connected pad. */
assert(!SDL_JoystickDetachVirtual(second));assert(sdl_host_service(&h,c));assert(!h.input.controller && h.input.await_select);
SDL_JoystickClose(other);
dune_controls_toggle(&h,c);h.controls.page=0;h.controls.selected=0;key(&h,c,SDLK_RIGHT);assert(h.input.instance==SDL_JoystickInstanceID(j));
/* A stick held at its limit on selection cannot move anything until centered. */
dune_controls_toggle(&h,c);h.stick_cursor_blocked=0;dune_stick_cursor(&h,c,1000);
assert(!h.input.armed && !h.dune_view.pointer_valid && !c->pad_buttons[0]);
assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_LEFTX,0));assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_LEFTY,0));
assert(sdl_host_service(&h,c));assert(h.input.armed);
/* Offset center, user calibration and proportional cursor movement. */
assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_LEFTX,14000));SDL_JoystickUpdate();
dune_controls_toggle(&h,c);h.controls.page=2;h.controls.selected=6;key(&h,c,SDLK_RETURN);
assert(h.input.center[0]==14000);dune_controls_toggle(&h,c);assert(sdl_host_service(&h,c));
assert(h.input.armed && !sdl_pad_value(&h.input,0));
h.dune_view.pointer_valid=1;h.dune_view.pointer_x=100;h.dune_view.pointer_y=100;h.stick_cursor_active=0;h.stick_cursor_blocked=0;
assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_LEFTX,32767));SDL_JoystickUpdate();
dune_stick_cursor(&h,c,2000);dune_stick_cursor(&h,c,2050);assert(h.dune_view.pointer_x>100 && h.dune_view.pointer_y==100);
h.input.invert_x=1;double before=h.stick_cursor_x;dune_stick_cursor(&h,c,2100);assert(h.stick_cursor_x<before);
h.input.focused=0;dune_stick_cursor(&h,c,2150);assert(!h.dune_view.pointer_valid);
h.input.focused=1;dune_stick_cursor(&h,c,2200);assert(!h.dune_view.pointer_valid);
assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_LEFTX,14000));SDL_JoystickUpdate();dune_stick_cursor(&h,c,2250);
dune_controls_write(&h);h.input.center[0]=0;dune_controls_read(&h);assert(h.input.center[0]==14000 && h.input.invert_x);
/* Mouse can be disabled independently; configuration remains reachable. */
h.input.mouse=0;h.dune_view.pointer_valid=0;e.type=SDL_MOUSEMOTION;e.motion.x=0;e.motion.y=0;
assert(SDL_PushEvent(&e)==1);assert(sdl_host_service(&h,c));assert(!h.dune_view.pointer_valid);
h.stick_cursor_active=1;h.stick_cursor_x=123;h.stick_cursor_y=234;
e.type=SDL_MOUSEMOTION;e.motion.x=900;e.motion.y=600;
assert(SDL_PushEvent(&e)==1);assert(sdl_host_service(&h,c));
assert(h.stick_cursor_active && h.stick_cursor_x==123 && h.stick_cursor_y==234);
key(&h,c,SDLK_F1);assert(h.controls.menu);key(&h,c,SDLK_F1);assert(!h.controls.menu);
/* Malformed configs are rejected atomically. */
FILE *f=fopen("gamepad.cfg","w");assert(f);fputs("ReArrakis controls 4\n1 0 1\n999",f);assert(!fclose(f));
dune_controls_read(&h);assert(h.input.center[0]==14000);
/* No arbitrary two-pad limit, even with identical virtual models. */
int collection[20];for(int i=0;i<20;i++){collection[i]=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,SDL_CONTROLLER_AXIS_MAX,SDL_CONTROLLER_BUTTON_MAX,0);assert(collection[i]>=0);}
for(int i=0;i<20;i++){sdl_pad_cycle(&h.input,1);assert(h.input.instance==SDL_JoystickGetDeviceInstanceID(collection[i]));}
sdl_pad_cycle(&h.input,1);assert(h.input.instance==SDL_JoystickInstanceID(j));
for(int i=19;i>=0;i--)assert(!SDL_JoystickDetachVirtual(collection[i]));
assert(!SDL_JoystickDetachVirtual(device));assert(sdl_host_service(&h,c));assert(!h.input.controller && !c->pad_buttons[0]);
''')
