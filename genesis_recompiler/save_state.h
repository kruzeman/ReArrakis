/* Local snapshots for this runtime/ABI. Host pointers are never restored. */
#ifndef GENESIS_SAVE_STATE_H
#define GENESIS_SAVE_STATE_H
static uint64_t state_hash(const void *data,size_t size) {
    const uint8_t *p=data;uint64_t hash=UINT64_C(14695981039346656037);
    while(size--){hash^=*p++;hash*=UINT64_C(1099511628211);}return hash;
}
/* ponytail: native CPU layout, gated by runtime fingerprint and ABI; use an
   explicit field format if cross-version/architecture state exchange is needed. */
typedef struct {
    char magic[8],runtime[65];
    uint64_t cpu_size,rom_size,rom_hash,cpu_hash,fm_size,fm_hash;
    uint32_t endian,pointer_size,features;
} StateHeader;
static unsigned state_features(void) {
    unsigned flags=0;
#ifdef GENESIS_AUDIO
    flags|=1;
#endif
#ifdef GENESIS_DUNE_MOUSE
    flags|=2;
#endif
    return flags;
}
static int state_save(CPU *c,const char *path) {
    if(c->fault || c->audio.wav)return 0;
    CPU *copy=malloc(sizeof *copy);if(!copy)return 0;
    memcpy(copy,c,sizeof *copy);copy->rom=NULL;copy->reason=NULL;
    copy->audio.fm=NULL;copy->audio.wav=NULL;
    StateHeader head={0};memcpy(head.magic,"RRSTATE1",8);
    memcpy(head.runtime,GENESIS_STATE_RUNTIME,sizeof head.runtime);
    head.cpu_size=sizeof *copy;head.rom_size=c->rom_size;head.rom_hash=state_hash(c->rom,c->rom_size);
    head.cpu_hash=state_hash(copy,sizeof *copy);head.endian=0x01020304;
    head.pointer_size=sizeof(void*);head.features=state_features();
    uint8_t *fm=NULL;int ok=1;
#ifdef GENESIS_AUDIO
    if(c->audio.fm){
        head.fm_size=genesis_ymfm_state_size(c->audio.fm);
        if(!head.fm_size || head.fm_size>1048576)ok=0;
        if(ok){fm=malloc((size_t)head.fm_size);ok=fm && genesis_ymfm_save_state(c->audio.fm,fm,(size_t)head.fm_size);}
    }
#endif
    if(ok)head.fm_hash=state_hash(fm,(size_t)head.fm_size);
    char temporary[256];int length=snprintf(temporary,sizeof temporary,"%s.tmp.%" PRIu64,path,(uint64_t)SDL_GetPerformanceCounter());
    if(length<0 || (size_t)length>=sizeof temporary)ok=0;
    FILE *f=ok?fopen(temporary,"wb"):NULL;if(!f)ok=0;
    if(f){
        ok=fwrite(&head,1,sizeof head,f)==sizeof head && fwrite(copy,1,sizeof *copy,f)==sizeof *copy;
        if(ok && head.fm_size)ok=fwrite(fm,1,(size_t)head.fm_size,f)==head.fm_size;
        if(fclose(f))ok=0;
        if(ok)ok=host_file_replace(temporary,path);
        if(!ok)remove(temporary);
    }
    free(fm);free(copy);return ok;
}
static int state_load(CPU *c,const char *path) {
    if(c->audio.wav)return 0;
    FILE *f=fopen(path,"rb");if(!f)return 0;
    StateHeader head={0};CPU *copy=NULL;uint8_t *fm=NULL;void *chip=NULL;
    int ok=fread(&head,1,sizeof head,f)==sizeof head && !memcmp(head.magic,"RRSTATE1",8) &&
        !memcmp(head.runtime,GENESIS_STATE_RUNTIME,sizeof head.runtime) && head.cpu_size==sizeof(CPU) &&
        head.endian==0x01020304 && head.pointer_size==sizeof(void*) && head.features==state_features() &&
        head.rom_size==c->rom_size && head.rom_hash==state_hash(c->rom,c->rom_size) && head.fm_size<=1048576;
    if(ok){copy=malloc(sizeof *copy);ok=copy && fread(copy,1,sizeof *copy,f)==sizeof *copy;}
    if(ok && head.fm_size){fm=malloc((size_t)head.fm_size);ok=fm && fread(fm,1,(size_t)head.fm_size,f)==head.fm_size;}
    if(ok)ok=fgetc(f)==EOF && !ferror(f) && head.cpu_hash==state_hash(copy,sizeof *copy) && head.fm_hash==state_hash(fm,(size_t)head.fm_size);
    if(fclose(f))ok=0;
    if(ok)ok=!copy->fault && !copy->rom && !copy->reason && !copy->audio.fm && !copy->audio.wav &&
        copy->psg.latch<8 && copy->eeprom.address<128 && copy->eeprom.page<=124 &&
        copy->eeprom.bits<=8 && copy->eeprom.phase<=EE_WAIT &&
        copy->rom_size==c->rom_size && copy->audio_mode>=AUDIO_STRICT && copy->audio_mode<=AUDIO_ON &&
        copy->vdp.frame_width<=320 && copy->vdp.frame_height<=240 && copy->vdp.pal<=1 &&
        copy->vdp.line<313 && copy->vdp.line_clock<3420 && copy->audio.read<AUDIO_RING_FRAMES &&
        copy->audio.count<=AUDIO_RING_FRAMES && copy->audio.wav_count<AUDIO_WAV_FRAMES &&
        !!copy->audio.initialized==!!head.fm_size;
#ifdef GENESIS_AUDIO
    if(ok && head.fm_size){chip=genesis_ymfm_load_state(fm,(size_t)head.fm_size);ok=chip!=NULL;}
#else
    if(head.fm_size)ok=0;
#endif
    if(ok){
        copy->rom=c->rom;copy->audio.fm=chip;copy->audio.playback=c->audio.playback;
        copy->audio.read=copy->audio.count=0;memset(copy->pad_buttons,0,sizeof copy->pad_buttons);
#ifdef GENESIS_AUDIO
        if(c->audio.fm)genesis_ymfm_destroy(c->audio.fm);
#endif
        memcpy(c,copy,sizeof *c);
    }
    free(fm);free(copy);return ok;
}
#endif
