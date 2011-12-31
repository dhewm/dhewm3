
#include <sys/wait.h>

#include "sys/sys_public.h"

// input
void	Sys_InitInput( void );
void	Sys_ShutdownInput( void );

void	IN_DeactivateMouse( void);
void	IN_ActivateMouse( void);

void	IN_Activate (bool active);
void	IN_Frame (void);

void	Sys_UpdateWindowMouseInputRect( void );
