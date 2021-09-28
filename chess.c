#include <stdio.h>
#include <stdlib.h>

//It is guaranteed by the C standard that the values of unset enum constants begin at zero and increase numerically for however many there are
//This is used extensively in this program as an optimization
//
//For instance for each enum piece p, p/2 will evaluate the same regardless of color to give the type of piece, and with the exception of empty, p&1 is false for white pieces and true for black
enum piece{whiteKing,blackKing,whiteQueen,blackQueen,whiteBishop,blackBishop,whiteKnight,blackKnight,whiteRook,blackRook,whitePawn,blackPawn,empty};

struct gameState{
  enum piece pieces[64];
};

//This board is reflected over the river so that a1, or zero is the first whiteRook's inital spot. It is the starting position for every traditional chess game.
const struct gameState initialState=(struct gameState){
	.pieces={	whiteRook,	whiteKnight,	whiteBishop,	whiteQueen,	whiteKing,	whiteBishop,	whiteKnight,	whiteRook,
			whitePawn,	whitePawn,	whitePawn,	whitePawn,	whitePawn,	whitePawn,	whitePawn,	whitePawn,
			empty,		empty,		empty,		empty,		empty,		empty,		empty,		empty,
			empty,		empty,		empty,		empty,		empty,		empty,		empty,		empty,
			empty,		empty,		empty,		empty,		empty,		empty,		empty,		empty,
			empty,		empty,		empty,		empty,		empty,		empty,		empty,		empty,
			blackPawn,	blackPawn,	blackPawn,	blackPawn,	blackPawn,	blackPawn,	blackPawn,	blackPawn,
			blackRook,	blackKnight,	blackBishop,	blackQueen,	blackKing,	blackBishop,	blackKnight,	blackRook	}
																				};

//For any gameState state and space s, state.pieces[s] gives the piece at s, for instance, state.pieces[d8] is an enum piece representing what fills space d8.
//Queens are in column d, kings are in column e, pawns are in rows 2 and 7
//Dummy was added and set to -1 to force compiler to implement enum space as a signed type so we can check for underflow 
enum space{a1,b1,c1,d1,e1,f1,g1,h1,a2,b2,c2,d2,e2,f2,g2,h2,a3,b3,c3,d3,e3,f3,g3,h3,a4,b4,c4,d4,e4,f4,g4,h4,a5,b5,c5,d5,e5,f5,g5,h5,a6,b6,c6,d6,e6,f6,g6,h6,a7,b7,c7,d7,e7,f7,g7,h7,a8,b8,c8,d8,e8,f8,g8,h8,dummy=-1};

//Moves have built-in linked-list capability
struct move{
  enum space from;
  enum space to;
  struct move *next;
};

void addMoveStack(struct move **moveStack,enum space from,enum space to){
  struct move *new = malloc(sizeof(struct move));
  *new = (struct move){.from = from,.to = to,.next = *moveStack};
  *moveStack = new;
}

//This function is called in findMoves and other functions it calls to check if a move is legal in that the destination does not contain a piece of the same color
//It adds the move to the stack if it is valid, and returns 0 when it hits something. The return value is only meaningful for pieces that can move infinitely
//This function assumes that a piece can move to an empty space or take a piece there, which is not true for pawns
_Bool addMove(struct move **moveStack,struct gameState *state,_Bool player,enum space from, enum space to){
  _Bool out=state->pieces[to] == empty;						//The loop always breaks unless the space found is empty
  if(out || (state->pieces[to] & 1) ^ player)					//If the space is empty or occupied by an opponent's piece
	addMoveStack(moveStack,from,to);
  return out;									//Return 0 if the loop should break
}

void infiniteNorth(struct move **moveStack,struct gameState *state,_Bool player,enum space s){
  for(enum space m = s+8; m <= h8; m+=8)
	if(!addMove(moveStack,state,player,s,m))
		break;
}
void infiniteSouth(struct move **moveStack,struct gameState *state,_Bool player,enum space s){
  for(enum space m = s-8; m >= a1; m-=8)
	if(!addMove(moveStack,state,player,s,m))
		break;
}
void infiniteEast(struct move **moveStack,struct gameState *state,_Bool player,enum space s){
  for(enum space m = s+1; m/8 == s/8; ++m)
	if(!addMove(moveStack,state,player,s,m))
		break;
}
void infiniteWest(struct move **moveStack,struct gameState *state,_Bool player,enum space s){
  for(enum space m = s-1; m/8 == s/8; --m)
	if(!addMove(moveStack,state,player,s,m))
		break;
}
void infiniteNortheast(struct move **moveStack,struct gameState *state,_Bool player,enum space s){
  for(enum space m = s+9; m<=h8 && (m&8) > (s&8); m+=9)
	if(!addMove(moveStack,state,player,s,m))
		break;
}
void infiniteSouthwest(struct move **moveStack,struct gameState *state,_Bool player,enum space s){
  for(enum space m = s-9;m >= a1 && (m&8) < (s&8); m-=9)
	if(!addMove(moveStack,state,player,s,m))
		break;
}
void infiniteNorthwest(struct move **moveStack,struct gameState *state,_Bool player,enum space s){
  for(enum space m = s+7; m <= h8 && (m&8) < (s&8); m+=7)
	if(!addMove(moveStack,state,player,s,m))
		break;
}
void infiniteSoutheast(struct move **moveStack,struct gameState *state,_Bool player,enum space s){
  for(enum space m = s-7; m >= a1 && (m&8) > (s&8); m-=7)
	if(!addMove(moveStack,state,player,s,m))
		break;
}

//This function finds all possible legal moves for all pieces on the board. It does not check to see if a move puts our king in check so that must be done later
struct move* findMoves(struct gameState *state,_Bool player){
  struct move *out=NULL;
  for(enum space s=a1;s<=h8;++s){
	if((state->pieces[s] & 1) ^ player)    //This skips spaces with opponent's pieces, and also skips empties during black's turn which makes no difference
		continue;
	switch(state->pieces[s]){
		case whiteKing:
		case blackKing:
			if(s+8 <= h8)
				addMove(&out,state,player,s,s+8);
			if(s >= 8)
				addMove(&out,state,player,s,s-8);
			if((s+1)/8 == s/8)
				addMove(&out,state,player,s,s+1);
			if((s-1)/8 == s/8)
				addMove(&out,state,player,s,s-1);
			if(s+9 <= h8 && (s+9)%8 > s%8)
				addMove(&out,state,player,s,s+9);
			if(s >= 9 && (s-9)%8 < s%8)
				addMove(&out,state,player,s,s-9);
			if(s+7 <= h8 && (s+7)%8 < s%8)
				addMove(&out,state,player,s,s+7);
			if(s >= 7 && (s-7)%8 > s%8)
				addMove(&out,state,player,s,s-7);
			break;
		case whiteQueen:
		case blackQueen:
			infiniteNorth(&out,state,player,s);
			infiniteSouth(&out,state,player,s);
			infiniteEast(&out,state,player,s);
			infiniteWest(&out,state,player,s);
			infiniteNortheast(&out,state,player,s);
			infiniteSouthwest(&out,state,player,s);
			infiniteNorthwest(&out,state,player,s);
			infiniteSoutheast(&out,state,player,s);
			break;
		case whiteBishop:
		case blackBishop:
			infiniteNortheast(&out,state,player,s);
			infiniteSouthwest(&out,state,player,s);
			infiniteNorthwest(&out,state,player,s);
			infiniteSoutheast(&out,state,player,s);
			break;
		case whiteKnight:
		case blackKnight:
			if(s%8 < 7){
				if(s+17 <= h8)
					addMove(&out,state,player,s,s+17);
				if(s-15 >= a1)
					addMove(&out,state,player,s,s-15);
			}
			if(s%8 < 6){
				if(s+10 <= h8)
					addMove(&out,state,player,s,s+10);
				if(s-6 >= a1)
					addMove(&out,state,player,s,s-6);
			}
			if(s%8 > 0){
			        if(s+15 <= h8)
					addMove(&out,state,player,s,s+15);
				if(s-17 >= a1)
					addMove(&out,state,player,s,s-17);
			}
			if(s%8 > 1){
			       	if(s+6 <= h8)
					addMove(&out,state,player,s,s+6);
				if(s-10 >= a1)
					addMove(&out,state,player,s,s-10);
			}
			break;
		case whiteRook:
		case blackRook:
			infiniteNorth(&out,state,player,s);
			infiniteSouth(&out,state,player,s);
			infiniteEast(&out,state,player,s);
			infiniteWest(&out,state,player,s);
			break;
		case whitePawn:
			if(state->pieces[s+8] == empty){				//There is always a cell 8 ahead of a white pawn because they become queens in row 8
				addMoveStack(&out,s,s+8);
				if(s/8 == a2/8 && state->pieces[s+16] == empty)		//If in starting position they can move two spaces if nothing is in the way
					addMoveStack(&out,s,s+16);
			}
			if((s%8) && (state->pieces[s+7] & 1) ^ player)			//If not on left edge and there is an enemy piece up and to the left
				addMoveStack(&out,s,s+7);
			if((s%8) != 7 && (state->pieces[s+9] & 1) ^ player)		//If not on right edge and there is an enemy piece up and to the right
				addMoveStack(&out,s,s+9);
			break;
		case blackPawn:
			if(state->pieces[s-8] == empty){
				addMoveStack(&out,s,s-8);
				if(s/8 == a7/8 && state->pieces[s-16] == empty)
					addMoveStack(&out,s,s-16);
			}
			if((s%8) != 7 && (state->pieces[s-7] & 1) ^ player)
				addMoveStack(&out,s,s-7);
			if((s%8) && (state->pieces[s-9] & 1) ^ player)
				addMoveStack(&out,s,s-9);
			break;
		case empty:			//Do nothing
			break;
	}
  }
  return out;
}

enum space findKing(struct gameState *state,_Bool player){
  for(enum space s=a1;s<=h8;++s)
	if(state->pieces[s] == player ? blackKing : whiteKing)
		return s;
  return dummy; //This should never happen
}

void move(struct gameState *state,struct move *m){
  state->pieces[m->to] = state->pieces[m->from];
  state->pieces[m->from] = blank;
}

void freeMoveList(struct move *list){
  while(list){
	struct move *hold = list->next;
	free(list);
	list = hold;
  }
}

int main(){
  struct gameState state = initialState;
  _Bool game=1;
  _Bool turn=0;		//White goes first
  while(game){
	struct move *myMoves = findMoves(&state,turn);
	if(!myMoves){
		printf("Stalemate\n");
		game=0;
		continue;
	}

  }
}


