/* Optional ROM-backed check: build/dune.c must have been generated first. */
#define GENESIS_NO_MAIN
#include "../build/dune.c"
#include <assert.h>
static CPU c;
static void frames(uint64_t end,int intro){
    while(!c.fault && c.vdp.frames<end){
        unsigned f=(unsigned)c.vdp.frames;
        c.pad_buttons[0]=intro && f<3350 ? (f>600 && f%180<8 ? PAD_START : f>900 && f%180>=90 && f%180<98 ? PAD_C : 0):0;
        machine_step(&c);
    }
    if(c.fault)fprintf(stderr,"%06x: %s\n",c.pc,c.reason);
    assert(!c.fault);
}
int main(void){
    c.rom=rom_data;c.rom_size=sizeof rom_data;c.sr=0x2700;c.audio_mode=AUDIO_ON;
    c.io_tx[0]=c.io_tx[1]=0xff;c.io_tx[2]=0xfb;
    c.a[7]=read_mem(&c,0,4);c.ssp=c.a[7];c.pc=read_mem(&c,4,4)&0xffffff;
    assert(audio_init(&c,NULL));frames(5000,1);
    assert(state_save(&c,"build/dune-test-state.grs"));frames(5030,0);
    uint64_t ram=state_hash(c.ram,sizeof c.ram),video=state_hash(c.vdp.frame,sizeof c.vdp.frame);
    uint64_t clock=c.master_cycles;uint32_t pc=c.pc;
    size_t size=genesis_ymfm_state_size(c.audio.fm);uint8_t *fm=malloc(size);assert(fm);
    assert(genesis_ymfm_save_state(c.audio.fm,fm,size));uint64_t sound=state_hash(fm,size);
    assert(state_load(&c,"build/dune-test-state.grs"));frames(5030,0);
    assert(ram==state_hash(c.ram,sizeof c.ram) && video==state_hash(c.vdp.frame,sizeof c.vdp.frame));
    assert(clock==c.master_cycles && pc==c.pc);
    assert(genesis_ymfm_save_state(c.audio.fm,fm,size) && sound==state_hash(fm,size));
    free(fm);assert(audio_finish(&c));remove("build/dune-test-state.grs");
    puts("Dune state: frame 5000 -> 5030 reproduced RAM, video, PC, clocks and FM state");return 0;
}
