There are totally four frames. 
Start is for player(s) to choose the mode they want to play. They are ¡®Practise¡¯ mode which single player mode and ¡®Race¡¯ mode which is two players racing mode. Choose different mode will be given different second frame as game stats. There is an ¡®Exit¡¯ button there to end the game at any time. When chose any mode, there will be three difficulty levels to choose. ¡®Easy¡¯, ¡®Medium¡¯ and ¡®Legendry¡¯ will also be shown in the coming game stats frame. As three difficulty levels are shown, there will be a ¡®Back¡¯ button to level player go back to change his game mode.
GameStats is racing mode game stats frame. It has a main clock in the center. Two sub-clocks to store players¡¯ records. Speed meters and direction monitors are below them. There are two ¡®Ready¡¯ button for players to click when they are ready. When two ¡®Ready¡¯ button are clicked, Main clock will have a count progress, changing background colour from ¡®red¡¯ to ¡®yellow¡¯ to ¡®green¡¯. When ¡®green¡¯ lights are on, game starts. With any player reach the finish line first, his time will be recorded on his own sub-clock and sub-clock back ground will be changed to ¡®green¡¯ to congratulate him. The other player¡¯s record will also be recorded when he reaches the end, but his sub-clock background will be changed to ¡®red¡¯ to let him know his lose.
Single is practise mode game stats frame. It is very simple like only left car of Racing mode is here in practise mode. Sub-clock is bigger and it can only be changed to green when reach the finish line. The only difference is there is a ¡®Try Again¡¯ button to let player try the game again to get greater record. 
Result is the frame shown the racing result. Player ONE which is left player and Player TWO is the left. Time will be record in form minute (2 digits), second (2 digits), millisecond (3 digits). WINNER will be shown on top textfield as ¡°Player ONE¡±, ¡°Player TWO¡± or ¡°TIE¡±. ¡°RePlay¡± button to go directly back to the beginning of the game. ¡°Exit¡± to end the game.
Record is the fame shown the practise record. Player best practise record will be shown in the middle form minute (2 digits), second (2 digits), millisecond (3 digits) as well. ¡°RePlay¡± button to go directly back to the beginning of the game. ¡°Exit¡± to end the game.


Direction are all keep in Straight now. Can be detected by ActionListener().
IN SINGLE.JAVA.
if(sec==2&&ms==574){lFinish=true;} to simulate car reach the end at 00:02:574

IN GAMESTATS.JAVA. 
if(sec==2&&ms==574){lFinish=true;} to simulate player one reaches the end at 00:02:574
if(sec==4&&ms==4){rFinish=true;}  to simulate player two reaches the end at 00:04:004

