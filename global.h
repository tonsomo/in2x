#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//LUDO:
# define INTEL_RENDER_NORMAL   0
# define INTEL_RENDER_X2       1
# define INTEL_RENDER_21	 2
# define INTEL_LAST_RENDER     2

# define MAX_PATH           256
# define INTEL_MAX_SAVE_STATE 5

  typedef struct Intel_t {
 
    char intel_save_used[INTEL_MAX_SAVE_STATE];
    char intel_save_name[MAX_PATH];
    char intel_home_dir[MAX_PATH];
    int  gp2x_screenshot_id;
    int  gp2x_cpu_clock;
    int  gp2x_reverse_analog;
    int  gp2x_active_joystick;
    int  intel_snd_enable;
    int  intel_render_mode;
    int  gp2x_skip_max_frame;
    int  gp2x_skip_cur_frame;
    int  intel_slow_down_max;
    int  volume;

  } Intel_t;

  extern Intel_t INTEL;
  
  extern int gp2x_screenshot_mode;

  extern void intel_global_init(void);

#endif
