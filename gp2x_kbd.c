/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include "gp2x.h"
#include "gp2x_virtualpad.h"
#include "gp2x_danzeff.h"
#include "global.h"
#include "config.h"

#include "macros.h"
#include "plat/plat.h"
#include "periph/periph.h"
#include "cp1600/cp1600.h"
#include "mem/mem.h"
#include "icart/icart.h"
#include "bincfg/bincfg.h"
#include "bincfg/legacy.h"
#include "pads/pads.h"
#include "pads/pads_cgc.h"
#include "pads/pads_intv2pc.h"
#include "gfx/gfx.h"
#include "snd/snd.h"
#include "ay8910/ay8910.h"
#include "demo/demo.h"
#include "stic/stic.h"
#include "speed/speed.h"
#include "debug/debug_.h"
#include "event/event.h"
#include "file/file.h"
#include "ivoice/ivoice.h"
#include "cfg/cfg.h"

#include "gp2x_kbd.h"
#include "gp2x_menu.h"

# define KBD_MIN_START_TIME   200
# define KBD_MAX_EVENT_TIME   100
# define KBD_MIN_PENDING_TIME 800
# define KBD_MIN_DANZEFF_TIME 1600
# define KBD_MIN_COMMAND_TIME 100

 unsigned int   keypadloc = 0;
 unsigned int   keypadon = 0;
 unsigned int   volchange = 0;
 unsigned int   curvolume = 0;

 extern SDL_Joystick *joystick;

 static int            loc_button_data[ KBD_MAX_BUTTONS ];
 static unsigned int   loc_last_event_time = 0;
 static char           loc_button_press[ KBD_MAX_BUTTONS ]; 
 static char           loc_button_release[ KBD_MAX_BUTTONS ]; 
 static unsigned int   loc_button_mask[ KBD_MAX_BUTTONS ] =
 {
   VK_UP         , /*  KBD_UP         */
   VK_UP_LEFT      , /*  KBD_RIGHT      */
   VK_LEFT       , /*  KBD_DOWN       */
   VK_DOWN_LEFT       , /*  KBD_LEFT       */
   VK_DOWN     ,
   VK_DOWN_RIGHT    ,
   VK_RIGHT  ,
   VK_UP_RIGHT   ,
   VK_START   , /*  KBD_TRIANGLE   */
   VK_SELECT     , /*  KBD_CIRCLE     */
   VK_FL      , /*  KBD_CROSS      */
   VK_FR     , /*  KBD_SQUARE     */
   VK_FA     , /*  KBD_SELECT     */
   VK_FB	,
   VK_FX      , /*  KBD_START      */
   VK_FY   , /*  KBD_LTRIGGER   */
   VK_VOL_UP,
   VK_VOL_DOWN,
   VK_TAT
 };

 static char loc_button_name[ KBD_ALL_BUTTONS ][10] =
 {
   "UP",
   "UP-LEFT",
   "LEFT",
   "DOWN-LEFT",
   "DOWN",
   "DOWN-RIGHT",
   "RIGHT",
   "UP-RIGHT",
   "START",
   "SELECT",
   "LTRIGGER",
   "RTRIGGER",
   "A",
   "B",
   "X",
   "Y",
   "VOLUP",
   "VOLDOWN",
   "CLICK"
 };

  extern cfg_t top_intv;

#define V(v) (&top_intv.v)

  intel_key_trans gp2x_intel_key_to_name[INTEL_MAX_KEY] =
  {
    { "RESET",      V(do_reset          ),  { 0,   ~0U },   { 0,   ~0U } },    /* INTEL_RESET,     */
    { "PD0L_KP1",   V(pad0.l[1]         ),  { 0,    ~0U  }, { 0,    0x81 } },  /* INTEL_PD0L_KP1,  */
    { "PD0L_KP2",   V(pad0.l[2]         ),  { 0,    ~0U  }, { 0,    0x41 } },  /* INTEL_PD0L_KP2,  */
    { "PD0L_KP3",   V(pad0.l[3]         ),  { 0,    ~0U  }, { 0,    0x21 } },  /* INTEL_PD0L_KP3,  */
    { "PD0L_KP4",   V(pad0.l[4]         ),  { 0,    ~0U  }, { 0,    0x82 } },  /* INTEL_PD0L_KP4,  */
    { "PD0L_KP5",   V(pad0.l[5]         ),  { 0,    ~0U  }, { 0,    0x42 } },  /* INTEL_PD0L_KP5,  */
    { "PD0L_KP6",   V(pad0.l[6]         ),  { 0,    ~0U  }, { 0,    0x22 } },  /* INTEL_PD0L_KP6,  */
    { "PD0L_KP7",   V(pad0.l[7]         ),  { 0,    ~0U  }, { 0,    0x84 } },  /* INTEL_PD0L_KP7,  */
    { "PD0L_KP8",   V(pad0.l[8]         ),  { 0,    ~0U  }, { 0,    0x44 } },  /* INTEL_PD0L_KP8,  */
    { "PD0L_KP9",   V(pad0.l[9]         ),  { 0,    ~0U  }, { 0,    0x24 } },  /* INTEL_PD0L_KP9,  */
    { "PD0L_KPC",   V(pad0.l[10]        ),  { 0,    ~0U  }, { 0,    0x88 } },  /* INTEL_PD0L_KPC,  */
    { "PD0L_KP0",   V(pad0.l[0]         ),  { 0,    ~0U  }, { 0,    0x48 } },  /* INTEL_PD0L_KP0,  */
    { "PD0L_KPE",   V(pad0.l[11]        ),  { 0,    ~0U  }, { 0,    0x28 } },  /* INTEL_PD0L_KPE,  */
    { "PD0L_A_T",   V(pad0.l[12]        ), { 0,    ~0U  }, { 0,    0xA0 } },   /* INTEL_PD0L_A_T,  */
    { "PD0L_A_L",   V(pad0.l[13]        ), { 0,    ~0U  }, { 0,    0x60 } },   /* INTEL_PD0L_A_L,  */
    { "PD0L_A_R",   V(pad0.l[14]        ), { 0,    ~0U  }, { 0,    0xC0 } },   /* INTEL_PD0L_A_R,  */
    { "PD0L_D_E",   V(pad0.l[15]        ), { ~1,   ~0U  }, { 0,    1  } },     /* INTEL_PD0L_D_E,  */
    { "PD0L_D_ENE", V(pad0.l[15]        ), { ~3,   ~0U  }, { 0,    3  } },     /* INTEL_PD0L_D_ENE,*/
    { "PD0L_D_NE",  V(pad0.l[15]        ), { ~2,   ~0U  }, { 0,    2  } },     /* INTEL_PD0L_D_NE, */
    { "PD0L_D_NNE", V(pad0.l[15]        ), { ~6,   ~0U  }, { 0,    6  } },     /* INTEL_PD0L_D_NNE,*/
    { "PD0L_D_N",   V(pad0.l[15]        ), { ~4,   ~0U  }, { 0,    4  } },     /* INTEL_PD0L_D_N,  */
    { "PD0L_D_NNW", V(pad0.l[15]        ), { ~12,  ~0U  }, { 0,    12 } },     /* INTEL_PD0L_D_NNW,*/
    { "PD0L_D_NW",  V(pad0.l[15]        ), { ~8,   ~0U  }, { 0,    8  } },     /* INTEL_PD0L_D_NW, */
    { "PD0L_D_WNW", V(pad0.l[15]        ), { ~24,  ~0U  }, { 0,    24 } },     /* INTEL_PD0L_D_WNW,*/
    { "PD0L_D_W",   V(pad0.l[15]        ), { ~16,  ~0U  }, { 0,    16 } },     /* INTEL_PD0L_D_W,  */
    { "PD0L_D_WSW", V(pad0.l[15]        ), { ~48,  ~0U  }, { 0,    48 } },     /* INTEL_PD0L_D_WSW,*/
    { "PD0L_D_SW",  V(pad0.l[15]        ), { ~32,  ~0U  }, { 0,    32 } },     /* INTEL_PD0L_D_SW, */
    { "PD0L_D_SSW", V(pad0.l[15]        ), { ~96,  ~0U  }, { 0,    96 } },     /* INTEL_PD0L_D_SSW,*/
    { "PD0L_D_S",   V(pad0.l[15]        ), { ~64,  ~0U  }, { 0,    64 } },     /* INTEL_PD0L_D_S,  */
    { "PD0L_D_SSE", V(pad0.l[15]        ), { ~192, ~0U  }, { 0,    192} },     /* INTEL_PD0L_D_SSE,*/
    { "PD0L_D_SE",  V(pad0.l[15]        ), { ~128, ~0U  }, { 0,    128} },     /* INTEL_PD0L_D_SE, */
    { "PD0L_J_E",   V(pad0.l[16]        ), { 0,    0    }, { 0,    1   }},     /* INTEL_PD0L_J_E,  */
    { "PD0L_J_ENE", V(pad0.l[16]        ), { 0,    0    }, { 0,    3   }},     /* INTEL_PD0L_J_ENE,*/
    { "PD0L_J_NE",  V(pad0.l[16]        ), { 0,    0    }, { 0,    2   }},     /* INTEL_PD0L_J_NE, */
    { "PD0L_J_NNE", V(pad0.l[16]        ), { 0,    0    }, { 0,    6   }},     /* INTEL_PD0L_J_NNE,*/
    { "PD0L_J_N",   V(pad0.l[16]        ), { 0,    0    }, { 0,    4   }},     /* INTEL_PD0L_J_N,  */
    { "PD0L_J_NNW", V(pad0.l[16]        ), { 0,    0    }, { 0,    12  }},     /* INTEL_PD0L_J_NNW,*/
    { "PD0L_J_NW",  V(pad0.l[16]        ), { 0,    0    }, { 0,    8   }},     /* INTEL_PD0L_J_NW, */
    { "PD0L_J_WNW", V(pad0.l[16]        ), { 0,    0    }, { 0,    24  }},     /* INTEL_PD0L_J_WNW,*/
    { "PD0L_J_W",   V(pad0.l[16]        ), { 0,    0    }, { 0,    16  }},     /* INTEL_PD0L_J_W,  */
    { "PD0L_J_WSW", V(pad0.l[16]        ), { 0,    0    }, { 0,    48  }},     /* INTEL_PD0L_J_WSW,*/
    { "PD0L_J_SW",  V(pad0.l[16]        ), { 0,    0    }, { 0,    32  }},     /* INTEL_PD0L_J_SW, */
    { "PD0L_J_SSW", V(pad0.l[16]        ), { 0,    0    }, { 0,    96  }},     /* INTEL_PD0L_J_SSW,*/
    { "PD0L_J_S",   V(pad0.l[16]        ), { 0,    0    }, { 0,    64  }},     /* INTEL_PD0L_J_S,  */
    { "PD0L_J_SSE", V(pad0.l[16]        ), { 0,    0    }, { 0,    192 }},     /* INTEL_PD0L_J_SSE,*/
    { "PD0L_J_SE",  V(pad0.l[16]        ), { 0,    0    }, { 0,    128 }},     /* INTEL_PD0L_J_SE, */
    { "PD0L_J_ESE", V(pad0.l[16]        ), { 0,    0    }, { 0,    129 }},     /* INTEL_PD0L_J_ESE,*/
    { "PD0R_KP1",   V(pad0.r[1]         ),  { 0,    ~0U  }, { 0,    0x81 } },  /* INTEL_PD0R_KP1,  */
    { "PD0R_KP2",   V(pad0.r[2]         ),  { 0,    ~0U  }, { 0,    0x41 } },  /* INTEL_PD0R_KP2,  */
    { "PD0R_KP3",   V(pad0.r[3]         ),  { 0,    ~0U  }, { 0,    0x21 } },  /* INTEL_PD0R_KP3,  */
    { "PD0R_KP4",   V(pad0.r[4]         ),  { 0,    ~0U  }, { 0,    0x82 } },  /* INTEL_PD0R_KP4,  */
    { "PD0R_KP5",   V(pad0.r[5]         ),  { 0,    ~0U  }, { 0,    0x42 } },  /* INTEL_PD0R_KP5,  */
    { "PD0R_KP6",   V(pad0.r[6]         ),  { 0,    ~0U  }, { 0,    0x22 } },  /* INTEL_PD0R_KP6,  */
    { "PD0R_KP7",   V(pad0.r[7]         ),  { 0,    ~0U  }, { 0,    0x84 } },  /* INTEL_PD0R_KP7,  */
    { "PD0R_KP8",   V(pad0.r[8]         ),  { 0,    ~0U  }, { 0,    0x44 } },  /* INTEL_PD0R_KP8,  */
    { "PD0R_KP9",   V(pad0.r[9]         ),  { 0,    ~0U  }, { 0,    0x24 } },  /* INTEL_PD0R_KP9,  */
    { "PD0R_KPC",   V(pad0.r[10]        ),  { 0,    ~0U  }, { 0,    0x88 } },  /* INTEL_PD0R_KPC,  */
    { "PD0R_KP0",   V(pad0.r[0]         ),  { 0,    ~0U  }, { 0,    0x48 } },  /* INTEL_PD0R_KP0,  */
    { "PD0R_KPE",   V(pad0.r[11]        ),  { 0,    ~0U  }, { 0,    0x28 } },  /* INTEL_PD0R_KPE,  */
    { "PD0R_A_T",   V(pad0.r[12]        ), { 0,    ~0U  }, { 0,    0xA0 } },   /* INTEL_PD0R_A_T,  */
    { "PD0R_A_L",   V(pad0.r[13]        ), { 0,    ~0U  }, { 0,    0x60 } },   /* INTEL_PD0R_A_L,  */
    { "PD0R_A_R",   V(pad0.r[14]        ), { 0,    ~0U  }, { 0,    0xC0 } },   /* INTEL_PD0R_A_R,  */
    { "PD0R_D_E",   V(pad0.r[15]        ), { ~1,   ~0U  }, { 0,    1  } },     /* INTEL_PD0R_D_E,  */
    { "PD0R_D_ENE", V(pad0.r[15]        ), { ~3,   ~0U  }, { 0,    3  } },     /* INTEL_PD0R_D_ENE,*/
    { "PD0R_D_NE",  V(pad0.r[15]        ), { ~2,   ~0U  }, { 0,    2  } },     /* INTEL_PD0R_D_NE, */
    { "PD0R_D_NNE", V(pad0.r[15]        ), { ~6,   ~0U  }, { 0,    6  } },     /* INTEL_PD0R_D_NNE,*/
    { "PD0R_D_N",   V(pad0.r[15]        ), { ~4,   ~0U  }, { 0,    4  } },     /* INTEL_PD0R_D_N,  */
    { "PD0R_D_NNW", V(pad0.r[15]        ), { ~12,  ~0U  }, { 0,    12 } },     /* INTEL_PD0R_D_NNW,*/
    { "PD0R_D_NW",  V(pad0.r[15]        ), { ~8,   ~0U  }, { 0,    8  } },     /* INTEL_PD0R_D_NW, */
    { "PD0R_D_WNW", V(pad0.r[15]        ), { ~24,  ~0U  }, { 0,    24 } },     /* INTEL_PD0R_D_WNW,*/
    { "PD0R_D_W",   V(pad0.r[15]        ), { ~16,  ~0U  }, { 0,    16 } },     /* INTEL_PD0R_D_W,  */
    { "PD0R_D_WSW", V(pad0.r[15]        ), { ~48,  ~0U  }, { 0,    48 } },     /* INTEL_PD0R_D_WSW,*/
    { "PD0R_D_SW",  V(pad0.r[15]        ), { ~32,  ~0U  }, { 0,    32 } },     /* INTEL_PD0R_D_SW, */
    { "PD0R_D_SSW", V(pad0.r[15]        ), { ~96,  ~0U  }, { 0,    96 } },     /* INTEL_PD0R_D_SSW,*/
    { "PD0R_D_S",   V(pad0.r[15]        ), { ~64,  ~0U  }, { 0,    64 } },     /* INTEL_PD0R_D_S,  */
    { "PD0R_D_SSE", V(pad0.r[15]        ), { ~192, ~0U  }, { 0,    192} },     /* INTEL_PD0R_D_SSE,*/
    { "PD0R_D_SE",  V(pad0.r[15]        ), { ~128, ~0U  }, { 0,    128} },     /* INTEL_PD0R_D_SE, */
    { "PD0R_J_E",   V(pad0.r[16]        ), { 0,    0    }, { 0,    1   }},     /* INTEL_PD0R_J_E,  */
    { "PD0R_J_ENE", V(pad0.r[16]        ), { 0,    0    }, { 0,    3   }},     /* INTEL_PD0R_J_ENE,*/
    { "PD0R_J_NE",  V(pad0.r[16]        ), { 0,    0    }, { 0,    2   }},     /* INTEL_PD0R_J_NE, */
    { "PD0R_J_NNE", V(pad0.r[16]        ), { 0,    0    }, { 0,    6   }},     /* INTEL_PD0R_J_NNE,*/
    { "PD0R_J_N",   V(pad0.r[16]        ), { 0,    0    }, { 0,    4   }},     /* INTEL_PD0R_J_N,  */
    { "PD0R_J_NNW", V(pad0.r[16]        ), { 0,    0    }, { 0,    12  }},     /* INTEL_PD0R_J_NNW,*/
    { "PD0R_J_NW",  V(pad0.r[16]        ), { 0,    0    }, { 0,    8   }},     /* INTEL_PD0R_J_NW, */
    { "PD0R_J_WNW", V(pad0.r[16]        ), { 0,    0    }, { 0,    24  }},     /* INTEL_PD0R_J_WNW,*/
    { "PD0R_J_W",   V(pad0.r[16]        ), { 0,    0    }, { 0,    16  }},     /* INTEL_PD0R_J_W,  */
    { "PD0R_J_WSW", V(pad0.r[16]        ), { 0,    0    }, { 0,    48  }},     /* INTEL_PD0R_J_WSW,*/
    { "PD0R_J_SW",  V(pad0.r[16]        ), { 0,    0    }, { 0,    32  }},     /* INTEL_PD0R_J_SW, */
    { "PD0R_J_SSW", V(pad0.r[16]        ), { 0,    0    }, { 0,    96  }},     /* INTEL_PD0R_J_SSW,*/
    { "PD0R_J_S",   V(pad0.r[16]        ), { 0,    0    }, { 0,    64  }},     /* INTEL_PD0R_J_S,  */
    { "PD0R_J_SSE", V(pad0.r[16]        ), { 0,    0    }, { 0,    192 }},     /* INTEL_PD0R_J_SSE,*/
    { "PD0R_J_SE",  V(pad0.r[16]        ), { 0,    0    }, { 0,    128 }},     /* INTEL_PD0R_J_SE, */
    { "PD0R_J_ESE", V(pad0.r[16]        ), { 0,    0    }, { 0,    129 }}      /* INTEL_PD0R_J_ESE,*/
 };

 static int loc_default_mapping[ KBD_ALL_BUTTONS ] = {
   INTEL_PD0L_J_N     , /*  KBD_UP         	*/
   INTEL_PD0L_J_NW    , /*  KBD_UPLEFT      	*/
   INTEL_PD0L_J_W     , /*  KBD_LEFT       	*/
   INTEL_PD0L_J_SW    , /*  KBD_DOWNLEFT       	*/
   INTEL_PD0L_J_S     , /*  KBD_DOWN		*/
   INTEL_PD0L_J_SE    , /*  KBD_DOWNRIGHT		*/
   INTEL_PD0L_J_E     , /*  KBD_RIGHT		*/
   INTEL_PD0L_J_NE    , /*  KBD_UPRIGHT		*/
   -1                 , /*  KBD_SELECT     	*/
   -1                 , /*  KBD_START      	*/
   INTEL_RESET        , /*  KBD_LTRIGGER   	*/
   INTEL_PD0L_KPE     , /*  KBD_RTRIGGER   	*/
   INTEL_PD0L_KP1     , /*  KBD_A     		*/
   INTEL_PD0L_A_R     , /*  KBD_B     		*/
   INTEL_PD0L_A_L     , /*  KBD_X      		*/
   INTEL_PD0L_A_T     , /*  KBD_Y   		*/
   -1			    ,
   -1			    ,
   -1                   /*  KBD_CLICK */
  };

# define KBD_MAX_ENTRIES   93

  int kbd_layout[KBD_MAX_ENTRIES][2] = {
    /* Key            Ascii */
    { INTEL_RESET,       DANZEFF_ESC  },
    { INTEL_PD0L_KP1,    '1'  },
    { INTEL_PD0L_KP2,    '2'  },
    { INTEL_PD0L_KP3,    '3'  },
    { INTEL_PD0L_KP4,    '4'  },
    { INTEL_PD0L_KP5,    '5'  },
    { INTEL_PD0L_KP6,    '6'  },
    { INTEL_PD0L_KP7,    '7'  },
    { INTEL_PD0L_KP8,    '8'  },
    { INTEL_PD0L_KP9,    '9'  },
    { INTEL_PD0L_KPC,    'C'  },
    { INTEL_PD0L_KP0,    '0'  },
    { INTEL_PD0L_KPE,    'E'  },
    { INTEL_PD0L_A_T,    'T' },
    { INTEL_PD0L_A_L,    'L' },
    { INTEL_PD0L_A_R,    'R' },
    { INTEL_PD0L_D_E,    -1  },
    { INTEL_PD0L_D_ENE,  -1  },
    { INTEL_PD0L_D_NE,   -1  },
    { INTEL_PD0L_D_NNE,  -1  },
    { INTEL_PD0L_D_N,    -1  },
    { INTEL_PD0L_D_NNW,  -1  },
    { INTEL_PD0L_D_NW,   -1  },
    { INTEL_PD0L_D_WNW,  -1  },
    { INTEL_PD0L_D_W,    -1  },
    { INTEL_PD0L_D_WSW,  -1  },
    { INTEL_PD0L_D_SW,   -1  },
    { INTEL_PD0L_D_SSW,  -1  },
    { INTEL_PD0L_D_S,    -1  },
    { INTEL_PD0L_D_SSE,  -1  },
    { INTEL_PD0L_D_SE,   -1  },
    { INTEL_PD0L_J_E,    -1  },
    { INTEL_PD0L_J_ENE,  -1  },
    { INTEL_PD0L_J_NE,   -1  },
    { INTEL_PD0L_J_NNE,  -1  },
    { INTEL_PD0L_J_N,    -1  },
    { INTEL_PD0L_J_NNW,  -1  },
    { INTEL_PD0L_J_NW,   -1  },
    { INTEL_PD0L_J_WNW   -1  },
    { INTEL_PD0L_J_W,    -1  },
    { INTEL_PD0L_J_WSW,  -1  },
    { INTEL_PD0L_J_SW,   -1  },
    { INTEL_PD0L_J_SSW   -1  },
    { INTEL_PD0L_J_S,    -1  },
    { INTEL_PD0L_J_SSE,  -1  },
    { INTEL_PD0L_J_SE,   -1  },
    { INTEL_PD0L_J_ESE,  -1  },
    { INTEL_PD0R_KP1,    -1  },
    { INTEL_PD0R_KP2,    -1  },
    { INTEL_PD0R_KP3,    -1  },
    { INTEL_PD0R_KP4,    -1  },
    { INTEL_PD0R_KP5,    -1  },
    { INTEL_PD0R_KP6,    -1  },
    { INTEL_PD0R_KP7,    -1  },
    { INTEL_PD0R_KP8,    -1  },
    { INTEL_PD0R_KP9,    -1  },
    { INTEL_PD0R_KPC,    -1  },
    { INTEL_PD0R_KP0,    -1  },
    { INTEL_PD0R_KPE,    -1  },
    { INTEL_PD0R_A_T,    -1  },
    { INTEL_PD0R_A_L,    -1  },
    { INTEL_PD0R_A_R,    -1  },
    { INTEL_PD0R_D_E,    -1  },
    { INTEL_PD0R_D_ENE,  -1  },
    { INTEL_PD0R_D_NE,   -1  },
    { INTEL_PD0R_D_NNE,  -1  },
    { INTEL_PD0R_D_N,    -1  },
    { INTEL_PD0R_D_NNW,  -1  },
    { INTEL_PD0R_D_NW,   -1  },
    { INTEL_PD0R_D_WNW,  -1  },
    { INTEL_PD0R_D_W,    -1  },
    { INTEL_PD0R_D_WSW,  -1  },
    { INTEL_PD0R_D_SW,   -1  },
    { INTEL_PD0R_D_SSW,  -1  },
    { INTEL_PD0R_D_S,    -1  },
    { INTEL_PD0R_D_SSE,  -1  },
    { INTEL_PD0R_D_SE,   -1  },
    { INTEL_PD0R_J_E,    -1  },
    { INTEL_PD0R_J_ENE,  -1  },
    { INTEL_PD0R_J_NE,   -1  },
    { INTEL_PD0R_J_NNE,  -1  },
    { INTEL_PD0R_J_N,    -1  },
    { INTEL_PD0R_J_NNW,  -1  },
    { INTEL_PD0R_J_NW,   -1  },
    { INTEL_PD0R_J_WNW   -1  },
    { INTEL_PD0R_J_W,    -1  },
    { INTEL_PD0R_J_WSW,  -1  },
    { INTEL_PD0R_J_SW,   -1  },
    { INTEL_PD0R_J_SSW   -1  },
    { INTEL_PD0R_J_S,    -1  },
    { INTEL_PD0R_J_SSE,  -1  },
    { INTEL_PD0R_J_SE,   -1  },
    { INTEL_PD0R_J_ESE,  -1  } 
  };

 int gp2x_kbd_mapping[ KBD_ALL_BUTTONS ];

 static int danzeff_intel_key     = 0;
 static int danzeff_intel_pending = 0;
 static int danzeff_mode        = 0;

 char command_keys[ 128 ];
 static int command_mode        = 0;
 static int command_index       = 0;
 static int command_size        = 0;
 static int command_intel_pending = 0;
 static int command_intel_key     = 0;

  extern uint_32 event_count;
  extern int keypadID;

void
gp2x_kbd_run_command(char *Command)
{
  strncpy(command_keys, Command, 128);
  command_size  = strlen(Command);
  command_index = 0;

  command_intel_key     = 0;
  command_intel_pending = 0;
  command_mode        = 1;
}

void
gp2x_kbd_run_command_reset()
{
  command_keys[0] = DANZEFF_ESC;
  command_keys[1] = 0;
  command_size  = 1;
  command_index = 0;

  command_intel_key     = 0;
  command_intel_pending = 0;
  command_mode        = 1;
}

int
intel_key_event(int intel_id, int button_pressed)
{
	uint_32 *pvalue;

	if (INTEL.gp2x_active_joystick) {
		fprintf(stdout,"active_joystick\n");
		fflush(stdout);
		if ((intel_id >= INTEL_PD0L_KP1) && (intel_id <= INTEL_PD0L_J_ESE)) {
      		intel_id += INTEL_PD0R_KP1 - INTEL_PD0L_KP1;
    		}
  	}

  	if ((intel_id >= 0) && (intel_id < INTEL_MAX_KEY)) {
    		pvalue = gp2x_intel_key_to_name[intel_id].pvalue;
    		if (pvalue != NULL) {
      		*pvalue &= gp2x_intel_key_to_name[intel_id].mask_and[button_pressed];
      		*pvalue |= gp2x_intel_key_to_name[intel_id].mask_or[button_pressed];
       		event_count++;
    		}
  	}
  	return 0;
}

int 
intel_kbd_reset()
{
  uint_32 *pvalue;
  int      intel_id;


  for (intel_id = INTEL_PD0L_KP1; intel_id < INTEL_MAX_KEY; intel_id++) {
    pvalue = gp2x_intel_key_to_name[intel_id].pvalue;
    if (pvalue != NULL) {
      *pvalue &= gp2x_intel_key_to_name[intel_id].mask_and[0];
      *pvalue |= gp2x_intel_key_to_name[intel_id].mask_or[0];
    }
  }
  return 0;
}

int
intel_get_key_from_ascii(int key_ascii)
{
  int index;
  for (index = 0; index < KBD_MAX_ENTRIES; index++) {
   if (kbd_layout[index][1] == key_ascii) return kbd_layout[index][0];
  }
  return -1;
}

int
gp2x_kbd_reset_mapping(void)
{
  memcpy(gp2x_kbd_mapping, loc_default_mapping, sizeof(loc_default_mapping));
  return 0;
}

int
gp2x_kbd_load_mapping(char *kbd_filename)
{
  char     Buffer[512];
  FILE    *KbdFile;
  char    *Scan;
  int      tmp_mapping[KBD_ALL_BUTTONS];
  int      intel_key_id = 0;
  int      kbd_id = 0;
  int      error = 0;

  memcpy(tmp_mapping, loc_default_mapping, sizeof(loc_default_mapping));

  KbdFile = fopen(kbd_filename, "r");
  error   = 1;

  if (KbdFile != (FILE*)0) {

    while (fgets(Buffer,512,KbdFile) != (char *)0) {

      Scan = strchr(Buffer,'\n');
      if (Scan) *Scan = '\0';
      /* For this #@$% of windows ! */
      Scan = strchr(Buffer,'\r');
      if (Scan) *Scan = '\0';
      if (Buffer[0] == '#') continue;

      Scan = strchr(Buffer,'=');
      if (! Scan) continue;

      *Scan = '\0';
      intel_key_id = atoi(Scan + 1);

      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,loc_button_name[kbd_id])) {
          tmp_mapping[kbd_id] = intel_key_id;
          break;
        }
      }
    }

    error = 0;
    fclose(KbdFile);
  }

  memcpy(gp2x_kbd_mapping, tmp_mapping, sizeof(gp2x_kbd_mapping));

  return error;
}

int
gp2x_kbd_save_mapping(char *kbd_filename)
{
  FILE    *KbdFile;
  int      kbd_id = 0;
  int      error = 0;

  KbdFile = fopen(kbd_filename, "w");
  error   = 1;

  if (KbdFile != (FILE*)0) {

    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", loc_button_name[kbd_id], gp2x_kbd_mapping[kbd_id]);
    }
    error = 0;
    fclose(KbdFile);
  }

  return error;
}

int
gp2x_kbd_enter_command()
{
  	unsigned long  c;

  	unsigned int command_key = 0;
  	int          intel_key     = 0;
  	int          key_event   = 0;

	c=gp2x_joystick_read();

  	if (command_intel_pending) 
  	{
    		if ((SDL_GetTicks() - loc_last_event_time) > KBD_MIN_COMMAND_TIME) {
      		loc_last_event_time = SDL_GetTicks();
      		command_intel_pending = 0;
      		intel_key_event(command_intel_key, 0);
    		}

    		return 0;
  	}

  	if ((SDL_GetTicks() - loc_last_event_time) > KBD_MIN_COMMAND_TIME) {
    		loc_last_event_time = SDL_GetTicks();

    		if (command_index >= command_size) {
      		command_mode  = 0;
      		command_index = 0;
      		command_size  = 0;

      		command_intel_pending = 0;
      		command_intel_key     = 0;

      		return 0;
    		}
  
    		command_key = command_keys[command_index++];
    		intel_key = intel_get_key_from_ascii(command_key);

    		if (intel_key != -1) {
      		command_intel_key     = intel_key;
      		command_intel_pending = 1;
      		intel_key_event(command_intel_key, 1);
    		}

    		return 1;
  	}

  	return 0;
}

int 
gp2x_kbd_is_danzeff_mode()
{
  return danzeff_mode;
}

int
gp2x_kbd_enter_danzeff()
{
	unsigned int danzeff_key = 0;
  	int          intel_key     = 0;
  	int          key_event   = 0;
  	unsigned long c;

	if (danzeff_intel_pending) 
  	{
    		if ((SDL_GetTicks() - loc_last_event_time) > KBD_MIN_PENDING_TIME) {
      		loc_last_event_time = SDL_GetTicks();
      		danzeff_intel_pending = 0;
      		intel_key_event(danzeff_intel_key, 0);
			danzeff_intel_key=-1;
			danzeff_mode=0;
    		}

    		return 0;
  	}

	intel_key = danzeff_intel_key; //intel_get_key_from_ascii(danzeff_key);

    	if (intel_key != -1) {
      	danzeff_intel_key     = intel_key;
      	danzeff_intel_pending = 1;
      	intel_key_event(danzeff_intel_key, 1);
    	}

    	return 1;
}

int
intel_decode_key(int gp2x_b, int button_pressed)
{
  	int wake = 0;
	static Uint32 last_time=0;

  	if (gp2x_b == KBD_SELECT) {
     		if (button_pressed)
		{
			if(keypadon==1)
				keypadon=0;
			else
				keypadon=1;
			//gp2x_kbd_enter_danzeff();
			//danzeff_mode=keypadon;
		}
  	} else if (gp2x_b == KBD_START) {
    		if (button_pressed) {
      		gp2x_main_menu(1);
			gp2x_kbd_wait_no_button();
			kbd_reset_button_status();
      		gp2x_init_keyboard();
    		}
	} else if (keypadon==1)
	{
		if(SDL_GetTicks()-last_time>400)
		{
			last_time=SDL_GetTicks();

			int oldloc=keypadloc;
		
			if(gp2x_b == KBD_LEFT)
			{
				if(keypadloc==0)
					keypadloc=11;
				else
					keypadloc-=1;
				printf("loc: %i\n",keypadloc);
			}
			else if(gp2x_b == KBD_RIGHT)
			{
				keypadloc+=1;
				if(keypadloc>11) keypadloc=0;
				printf("loc: %i\n",keypadloc);
			}
			else if(gp2x_b == KBD_UP)
			{
				keypadloc-=6;
				if(keypadloc<0) keypadloc+=12;
			}
			else if(gp2x_b == KBD_DOWN)
			{
				keypadloc+=6;
				if(keypadloc>11) keypadloc-=12;
			}

			if(gp2x_b == KBD_Y || gp2x_b == KBD_B || gp2x_b == KBD_X || gp2x_b == KBD_A) 
			{
  				int intel_key = 0;

				switch(keypadloc)
				{
					case 0:
  						intel_key=INTEL_PD0L_KP0;
						break;
					case 1:
  						intel_key=INTEL_PD0L_KP1;
						break;
					case 2:
  						intel_key=INTEL_PD0L_KP2;  
						break;
					case 3:
  						intel_key=INTEL_PD0L_KP3;   
						break;
					case 4:
  						intel_key=INTEL_PD0L_KP4;   
						break;
					case 5:
  						intel_key=INTEL_PD0L_KPC;  
						break;
					case 6:
  						intel_key=INTEL_PD0L_KP5;  
						break;
					case 7:
  						intel_key=INTEL_PD0L_KP6;  
						break;
					case 8:
  						intel_key=INTEL_PD0L_KP7;  
						break;
					case 9:
					 	intel_key=INTEL_PD0L_KP8;  
						break;
					case 10:
  						intel_key=INTEL_PD0L_KP9;
						break;
					case 11:
  						intel_key=INTEL_PD0L_KPE;  
						break;
					default:
						intel_key=-1;
				}

				fprintf(stdout,"Intel Key: %i = Keypad Loc: %i\n",intel_key,keypadloc);
				fflush(stdout);
    				if (intel_key != -1) {
      				loc_last_event_time = SDL_GetTicks();
      				danzeff_intel_key     = intel_key;
					danzeff_mode=1;
    				}
				keypadon=0;
				return 0;
			}
		}
  	} else {
    		if (gp2x_kbd_mapping[gp2x_b] != -1) {
      		wake = 1;
      		intel_key_event(gp2x_kbd_mapping[gp2x_b], button_pressed);
    		}
  	}
	return 0;
}

int 
kbd_reset_button_status(void)
{
  int b = 0;
  /* Reset Button status */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    loc_button_press[b]   = 0;
    loc_button_release[b] = 0;
  }
  gp2x_init_keyboard();
  return 0;
}

int
kbd_scan_keyboard(void)
{
	unsigned long c;
	int         event;
	int         b;
	int		jb[19];

	int new_Lx=0;
	int new_Ly=0;
	int old_Lx=0;
	int old_Ly=0;

	event = 0;
	//c=gp2x_joystick_read();

      SDL_JoystickUpdate();

	for(b=0;b<19;b++)
		jb[b]=SDL_JoystickGetButton(joystick, b);
    
      if ( jb[VK_START] != 0 ) {
            // Do something
      }

	//if((c&(GP2X_L|GP2X_R|GP2X_START))==(GP2X_L|GP2X_R|GP2X_START))
	//	SDL_Quit();

	if(jb[VK_VOL_UP]!=0 || jb[VK_VOL_DOWN]!=0)
	{
		int cvol = INTEL.volume;
		if(jb[VK_VOL_UP]!=0) {
			cvol++;
			if(cvol>100) cvol=100;
		} else {
			cvol--;
			if(cvol<0) cvol=0;
		}
		gp2x_sound_volume(cvol,cvol);
		volchange = SDL_GetTicks()+1500;
		curvolume=cvol;
		INTEL.volume=cvol;
	}

	if(keypadon==0)
	{
		if(jb[VK_LEFT]!=0) new_Lx=-1;
		if(jb[VK_RIGHT]!=0) new_Lx=1;
		if(jb[VK_UP]!=0) new_Ly=-1;
		if(jb[VK_DOWN]!=0) new_Ly=1;

		if(jb[VK_UP_LEFT]!=0) {
			intel_decode_key(KBD_LEFT,0);
			intel_decode_key(KBD_RIGHT,0);
			intel_decode_key(KBD_UP,0);
			intel_decode_key(KBD_DOWN,0);
			intel_decode_key(KBD_DOWNLEFT,0);
			intel_decode_key(KBD_DOWNRIGHT,0);
			intel_decode_key(KBD_UPRIGHT,0);
			intel_decode_key(KBD_UPLEFT,1);
		} else
		if(jb[VK_DOWN_LEFT]!=0) {
			intel_decode_key(KBD_LEFT,0);
			intel_decode_key(KBD_RIGHT,0);
			intel_decode_key(KBD_UP,0);
			intel_decode_key(KBD_DOWN,0);
			intel_decode_key(KBD_UPLEFT,0);
			intel_decode_key(KBD_DOWNRIGHT,0);
			intel_decode_key(KBD_UPRIGHT,0);
			intel_decode_key(KBD_DOWNLEFT,1);
		} else
		if(jb[VK_UP_RIGHT]!=0) {
			intel_decode_key(KBD_LEFT,0);
			intel_decode_key(KBD_RIGHT,0);
			intel_decode_key(KBD_UP,0);
			intel_decode_key(KBD_DOWN,0);
			intel_decode_key(KBD_DOWNLEFT,0);
			intel_decode_key(KBD_DOWNRIGHT,0);
			intel_decode_key(KBD_UPLEFT,0);
			intel_decode_key(KBD_UPRIGHT,1);
		} else
		if(jb[VK_DOWN_RIGHT]!=0) {
			intel_decode_key(KBD_LEFT,0);
			intel_decode_key(KBD_RIGHT,0);
			intel_decode_key(KBD_UP,0);
			intel_decode_key(KBD_DOWN,0);
			intel_decode_key(KBD_DOWNLEFT,0);
			intel_decode_key(KBD_UPLEFT,0);
			intel_decode_key(KBD_UPRIGHT,0);
			intel_decode_key(KBD_DOWNRIGHT,1);
		} else {
	  		if (new_Lx > 0) {
				intel_decode_key(KBD_DOWNLEFT,0);
				intel_decode_key(KBD_UPLEFT,0);
				intel_decode_key(KBD_UPRIGHT,0);
				intel_decode_key(KBD_DOWNRIGHT,0);
	    			intel_decode_key(KBD_LEFT , 0);
	    			intel_decode_key(KBD_RIGHT, 1);
	  		} else if (new_Lx < 0) {
				intel_decode_key(KBD_DOWNLEFT,0);
				intel_decode_key(KBD_UPLEFT,0);
				intel_decode_key(KBD_UPRIGHT,0);
				intel_decode_key(KBD_DOWNRIGHT,0);
				intel_decode_key(KBD_RIGHT, 0);
				intel_decode_key(KBD_LEFT , 1);
	  		}

	  		if (new_Ly > 0) {
				intel_decode_key(KBD_DOWNLEFT,0);
				intel_decode_key(KBD_UPLEFT,0);
				intel_decode_key(KBD_UPRIGHT,0);
				intel_decode_key(KBD_DOWNRIGHT,0);
	    			intel_decode_key(KBD_UP , 0);
    				intel_decode_key(KBD_DOWN   , 1);
	  		} else if (new_Ly < 0) {
				intel_decode_key(KBD_DOWNLEFT,0);
				intel_decode_key(KBD_UPLEFT,0);
				intel_decode_key(KBD_UPRIGHT,0);
				intel_decode_key(KBD_DOWNRIGHT,0);
				intel_decode_key(KBD_DOWN   , 0);
				intel_decode_key(KBD_UP , 1);
	  		}
		}
	}

  	for (b = 0; b < KBD_MAX_BUTTONS; b++)
  	{
    		if (jb[loc_button_mask[b]]) {
      		if (!(loc_button_data[loc_button_mask[b]])) {
      			loc_button_press[b] = 1;
        			event = 1;
      		}
    		} else {
      	if (loc_button_data[loc_button_mask[b]]) {
        		loc_button_release[b] = 1;
        		event = 1;
      	}
    	}
  }
  for(b=0;b<19;b++)
  	loc_button_data[b]=jb[b];

  return event;
}

void
kbd_wait_start(void)
{
 	while (1)
  	{
    		unsigned long c;
    		
    		c=gp2x_joystick_read();
            if(c&GP2X_START)
            {
                 break;
            }
  	}
}

void
gp2x_init_keyboard(void)
{
  intel_kbd_reset();
}

void
gp2x_kbd_wait_no_button(void)
{
	SDL_Event c;
	Uint32 last_time;

  	last_time = SDL_GetTicks();
  	while(SDL_GetTicks()-last_time<400){
  	};

	while(SDL_PollEvent(&c)){
	};
} 

int
gp2x_update_keys(void)
{
  	int         b;

  	static char first_time = 1;
  	static long first_time_stamp = -1;
  	static int release_pending = 0;

  	if (first_time) {
    		memcpy(gp2x_kbd_mapping, loc_default_mapping, sizeof(loc_default_mapping));
    		unsigned long c;

    		if (first_time_stamp == -1) first_time_stamp = SDL_GetTicks();
    		if ((!(c=gp2x_joystick_read())) && ((SDL_GetTicks() - first_time_stamp) < KBD_MIN_START_TIME)) return 0;
    
    		first_time      = 0;
    		release_pending = 0;

    		for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      		loc_button_release[b] = 0;
      		loc_button_press[b] = 0;
    		}

    		return 0;
  	}

  	if (command_mode) {
    		return gp2x_kbd_enter_command();
  	}

  	if (danzeff_mode) {
    		return gp2x_kbd_enter_danzeff();
  	}

  	if (release_pending)
  	{
    		release_pending = 0;
    		for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      		if (loc_button_release[b]) {
        			loc_button_release[b] = 0;
        			loc_button_press[b] = 0;
        			intel_decode_key(b, 0);
      		}
    		}
  	}

  	kbd_scan_keyboard();

  	/* check release event */
  	for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    		if (loc_button_release[b]) {
      	release_pending = 1;
      	break;
    	} 
  }
  /* check press event */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    if (loc_button_press[b]) {
      loc_button_press[b] = 0;
      release_pending     = 0;
      intel_decode_key(b, 1);
    }
  }

  return 0;
}
