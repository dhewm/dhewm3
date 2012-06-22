#ifndef   __PLAYERCURSOR_H__ 
#define   __PLAYERCURSOR_H__ 
/* 
 Created on 4/19/06 by Paul Reed 
 Draws a cross hair sprite 5 feet from the view origin along a line leading to where 
 the player's weapon is pointing 
 */ 
class idPlayerCursor { 
public: 
   idPlayerCursor(); 
   ~idPlayerCursor(); 
   void   Draw( const idVec3 &origin,const idMat3 &axis,const char *material ); 
public: 
   renderEntity_t      renderEnt; 
   qhandle_t         cursorHandle; 
   bool            created; 
public: 
   void   FreeCursor( void ); 
   bool   CreateCursor( idPlayer* player , const idVec3 &origin, const idMat3 &axis,const char *material); 
   void   UpdateCursor( idPlayer* player ,const idVec3 &origin, const idMat3 &axis); 

}; 
#endif   /* !_PLAYERCURSOR_H_ */ 