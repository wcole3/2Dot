//
//  lGameMethods.h
//  SDL_2dot_game
//
//
//  Methods to control game loop logic
//
//
//  Created by William Cole on 3/18/19.
//  Copyright © 2019 William Cole. All rights reserved.
//

#ifndef lGameMethods_h
#define lGameMethods_h
//forward declarations here

//method to main game loop
/**
    Perform the game loop logic
 
    @param globalQuit true if the user wants to quit
    @param fullRun true if the game mode is a full run
 */
void playGame(bool* globalQuit, bool fullRun);
//method to check if user wants to play again
/**
    Check if the user wants to play again
 
    @param globalQuit true if the user wants to quit
    @param time the finishing time
 
    @return true if the user wants to play again
 */
bool playAgain(bool* globalQuit, float time);
//method check if the player want to continue their run and to play next level
/**
 Check if user wants to play next level or current level again; context is determined in playGame
 
    @param globalQuit true if the user want to quit
    @param stageTime the time the previous stage was completed in
    @param currentRunTime the current time elapsed in the run
 
    @return true if the user wants to play the next level or this level again
 */
bool playLevel(bool* globalQuit, float stageTime, float currentRunTime);
//method to display the pregame instructions and ask for user to start the game
/**
    Render the pregame screen and wait for user ready
 
    @param globalQuit true if the user wants to quit
 */
void pregameSetup(bool* globalQuit);
//method to load the level after restart or level change
/**
    Load the level tiles and reset the player positions
 
    @param levelNumber the index of the level to setup
 */
void loadLevel(int levelNumber);


//the actual game loop
void playGame(bool* globalQuit, bool fullRun){
    loadLevel(currentLevel);
    if(*globalQuit != true){
        //the loop condition for playing the game
        bool played = true;
        //event to get user inputs
        SDL_Event e;
        //setup timer for physics
        lTimer frameTicker = lTimer();
        //need a seperate timer for countdown
        lTimer countdownTicker = lTimer();
        //load the starting text
        float startTime = 0;
        string startString = "Time: ";
        SDL_Color countdownColor = {0,0,0};
        gImageTextures[gCountdownText]->loadFromRenderedText(startString + std::to_string(startTime), countdownColor);
        countdownTicker.start();
        while(played){
            while(SDL_PollEvent(&e) != 0){
                //test for quit
                if(e.type == SDL_QUIT){
                    *globalQuit = true;
                    played = false;
                }
                //otherwise handle dot events
                //only handle player events if the player hasnt finished
                if(!player1.isFinished()){
                    player1.handleEvent(e);
                }
                if(!player2.isFinished()){
                    player2.handleEvent(e);
                }
                gWindow.handleEvent(e);
                //update the UI
                resizeUI(&e);
                if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE){
                    played = false;
                }
            }
            //here we render stuff
            //first move dot
            float time = (frameTicker.getTime() / 1000.f);
            player1.move(time, player2.getCollider(), player1.getLevelTiles());
            player1.setCamera(camera1);
            player2.move(time, player1.getCollider(), player2.getLevelTiles());
            player2.setCamera(camera2);
            //restart time
            frameTicker.start();
            //now render
            gWindow.render();
            SDL_RenderSetViewport(gWindow.getRenderer(), &player1Screen);
            //render background
            for(int i = 0; i < TOTAL_TILES[currentLevel]; ++i){
                //render each tile
                player1.getLevelTiles()[i]->render(gImageTextures[gTileSpriteSheet], camera1);
            }
            //check if controls need to be rendered
            if(!player1.hasMoved()){
                //render the controls
                for(int i = 0; i < (sizeof(gPlayerPrompt)/sizeof(gPlayerPrompt[0]))/2; ++i){
                    gPlayerPrompt[i]. render(80, 70 + (i * gPlayerPrompt[i].getHeight()));
                }
            }
            player1.render(camera1);
            player2.render(camera1);
            //render the second dots screen
            SDL_RenderSetViewport(gWindow.getRenderer(), &player2Screen);
            for(int i = 0; i < TOTAL_TILES[currentLevel]; ++i){
                player2.getLevelTiles()[i]->render(gImageTextures[gTileSpriteSheet], camera2);
            }
            //render player 2 controls
            if(!player2.hasMoved()){
                //render the controls
                for(int i = 0; i < (sizeof(gPlayerPrompt)/sizeof(gPlayerPrompt[0]))/2; ++i){
                    gPlayerPrompt[i+4]. render(camera2.w - 160, (camera2.h - 160) + (i * gPlayerPrompt[i+4].getHeight()));
                }
            }
            player1.render(camera2);
            player2.render(camera2);
            SDL_RenderSetViewport(gWindow.getRenderer(), NULL);
            SDL_RenderFillRect(gWindow.getRenderer(), &split);
            //now render the time remaining
            float runningTime = countdownTicker.getTime() / 1000.f;
            char timeBuffer [10];
            sprintf(timeBuffer, "%.3f", runningTime);
            //now rerender the countdown text
            gImageTextures[gCountdownText]->loadFromRenderedText(startString + timeBuffer, countdownColor);
            gImageTextures[gCountdownText]->render((gWindow.getWidth() - gImageTextures[gCountdownText]->getWidth())/2, 0);
            SDL_RenderPresent(gWindow.getRenderer());
            //or if the limit has passed or both players are done
            if((player1.isFinished() && player2.isFinished())){
                Mix_Volume(-1, 70);
                //pause music
                Mix_FadeOutMusic(200);
                float finishTime;
                countdownTicker.pause();
                //check if game was one or lost
                //game was finished
                finishTime = (countdownTicker.getTime() / 1000.f);
                countdownTicker.stop();
                if(fullRun){
                    //need to check if we have complete all stages or are continuing the run
                    if(currentLevel < TOTAL_LEVELS - 1){
                        runTime += finishTime;
                        //display the next level screen, stage finish time, and current run time
                        if(playLevel(globalQuit, finishTime, runTime)){
                            SDL_RenderClear(gWindow.getRenderer());
                            //rewind the music
                            Mix_FadeInMusic(gGameMusic, -1, 100);
                            Mix_RewindMusic();
                            Mix_SetMusicPosition(2.9);
                            //update level info and reset players
                            currentLevel += 1;
                            loadLevel(currentLevel);
                            player1.setCamera(camera1);
                            player2.setCamera(camera2);
                            countdownTicker.start();
                        }else{
                            //user doesnt want to play next level, quit to menu
                            SDL_RenderClear(gWindow.getRenderer());
                            currentLevel = 0;
                            played = false;
                        }
                    }
                    else if(currentLevel == TOTAL_LEVELS - 1){
                        //all runs completed
                        runTime += finishTime;
                        if(playAgain(globalQuit, runTime)){
                            countdownTicker.start();
                            //clear out splash screen
                            SDL_RenderClear(gWindow.getRenderer());
                            //rewind the music
                            Mix_FadeInMusic(gGameMusic, -1, 100);
                            Mix_RewindMusic();
                            Mix_SetMusicPosition(2.9);
                            //reset run info
                            runTime = 0;
                            currentLevel = 0;
                            loadLevel(currentLevel);
                            player1.setCamera(camera1);
                            player2.setCamera(camera2);
                        }else{
                            SDL_RenderClear(gWindow.getRenderer());
                            played = false;
                        }
                    }
                }else{
                    //mode is single level, check user wants to play again
                    //todo
                    if(playLevel(globalQuit, finishTime, finishTime)){//this is a hack, might change in future
                        SDL_RenderClear(gWindow.getRenderer());
                        //rewind the music
                        Mix_FadeInMusic(gGameMusic, -1, 100);
                        Mix_RewindMusic();
                        Mix_SetMusicPosition(2.9);
                        loadLevel(currentLevel);
                        player1.setCamera(camera1);
                        player2.setCamera(camera2);
                        runTime = 0;
                        countdownTicker.start();
                    }else{
                        SDL_RenderClear(gWindow.getRenderer());
                        played = false;
                    }
                }
            }
        }
        Mix_HaltMusic();
        //play menumusic
        Mix_VolumeMusic(30);
        Mix_FadeInMusic(gMenuMusic, -1,1000);
    }
}

//method to check if the user wants to play again, this got a lot bigger than I intended (TODO chop this up)
bool playAgain(bool* globalQuit, float time){
    //reset the letter textures
    lTimer delayTimer = lTimer();
    delayTimer.start();
    string defaultName[MAX_LETTERS] = {"N", "E", "W"};
    for(int i = 0; i < (sizeof(gLetters)/sizeof(gLetters[0])); ++i){
        gLetters[i].loadFromRenderedText(defaultName[i].c_str(), blue);
    }
    //box to highlight the current letter
    SDL_Rect currentLetter = {(3*gWindow.getWidth()) / 4, (gWindow.getHeight() / 7) + ((LEADERBOARD_LINES + 1) * (gWindow.getHeight() / 10)), (gWindow.getHeight() / 12), (gWindow.getHeight() / 10)};
    //index for the current letter
    int letterIndex = 0;
    //local text textures for the leaderboard
    lTexture lLeaderboardHeader = lTexture(gWindow.getRenderer());
    lTexture lUserTime = lTexture(gWindow.getRenderer());
    lTexture lPlayAgainPrompt = lTexture(gWindow.getRenderer());
    lTexture lEnterNewNamePrompt = lTexture(gWindow.getRenderer());
    lEnterNewNamePrompt.setFont(gFont);
    lPlayAgainPrompt.setFont(gFont);
    lUserTime.setFont(gFont);
    lLeaderboardHeader.setFont(gFont);
    string header = "Current Leaders";
    if(!lLeaderboardHeader.loadFromRenderedText(header.c_str(), black)){
        printf("Could not load header texture!\n");
    }
    //load play again prompt
    string playAgainPrompt = "Play again? (y/n)";
    if(!lPlayAgainPrompt.loadFromRenderedText(playAgainPrompt.c_str(), black)){
        printf("Could not load play again prompt!\n");
    }
    //load the enter new name prompt
    string newNamePrompt = "Enter new name: ";
    if(!lEnterNewNamePrompt.loadFromRenderedText(newNamePrompt.c_str(), blue)){
        printf("Could not load enter new name prompt!\n");
    }
    if(time > 0){
        //load a prompt of the users time if they won
        char userTime[10];
        sprintf(userTime, "%.3f", time);
        string newTime = "Your time: ";
        newTime += userTime;
        if(!lUserTime.loadFromRenderedText(newTime.c_str(), blue)){
            printf("Could not load user time prompt!\n");
        }
        
    }
    //need to chek for global closes
    bool playAgain = false;
    //variables for control loop
    bool done = false;
    SDL_Event e;
    //highlight box for new winner
    SDL_Rect highlightBox = {gWindow.getWidth()/4, gWindow.getHeight() / 7 , gWindow.getWidth() / 2 ,gWindow.getHeight() / 10};
    //check for new name entered
    bool newNameEntered = false;
    SDL_StartTextInput();
    //int index for a possible new high score
    int index = -1;
    //check for new entries
    bool highlight = newLeader(time, &index);
    if(highlight){
        //the user won
        Mix_PlayChannel(-1, gWinSound, 0);
        //change the highlight box y value to the correct one
        highlightBox.y += (index + 1) * highlightBox.h;
    }else{
        Mix_PlayChannel(-1, gLoseSound, 0);
    }
    while(!done){
        while(SDL_PollEvent(&e) != 0){
            if(e.type == SDL_QUIT){
                //if the user want to quit
                *globalQuit = true;
                done = true;
            }
            //handle window resizing of the ui
            gWindow.handleEvent(e);
            resizeUI(&e);
            if((e.type == SDL_KEYDOWN && !highlight) || (e.type == SDL_KEYDOWN && newNameEntered)){
                if(e.key.keysym.sym == SDLK_y){
                    playAgain = true;
                    done = true;
                    SDL_Delay(100);
                    if(highlight){
                        //TODO save leaderboard
                        saveLeaderboard(leaderboardFile);
                    }
                }
                if(e.key.keysym.sym == SDLK_n){
                    done = true;
                    SDL_Delay(100);
                    if(highlight){
                        //TODO save leaderboard
                        saveLeaderboard(leaderboardFile);
                    }
                }
            }
            //handle event if user is entering a name
            if(highlight && !newNameEntered && (delayTimer.getTime() > 1000)){
                getNewName(e, &currentLetter, globalQuit, &letterIndex, &newNameEntered, defaultName);
                delayTimer.pause();
            }
        }
        //display the chosen splash screen
        SDL_RenderClear(gWindow.getRenderer());
        SDL_Rect currentScreen = {0,0,gWindow.getWidth(), gWindow.getHeight()};
        SDL_Rect textLine = {gWindow.getWidth()/4, gWindow.getHeight() / 7 , gWindow.getWidth() / 2 ,gWindow.getHeight() / 10};
        gImageTextures[gWinSplash]->render(0, 0, NULL, &currentScreen);
        //render the leaderboard
        lLeaderboardHeader.render(textLine.x, textLine.y, NULL, &textLine);
        for(int i = 0; i < (sizeof(gLeaderboardEntry)/sizeof(gLeaderboardEntry[0])); ++i){
            gLeaderboardEntry[i].render(textLine.x, textLine.y + ((i + 1) * textLine.h), NULL, &textLine);
        }
        //render play again prompt
        lPlayAgainPrompt.render(textLine.x, (9 * gWindow.getHeight()) / 10, NULL, &textLine);
        //render the usertimes
        if(time > 0){
            if(highlight){
                //the users time is on the board
                SDL_SetRenderDrawColor(gWindow.getRenderer(), 100, 50, 50, 100);
                SDL_RenderFillRect(gWindow.getRenderer(), &highlightBox);
                if(!newNameEntered){
                    for(int i = 0; i < MAX_LETTERS; ++i){
                        gLetters[i].render(((3*gWindow.getWidth()) / 4) + (i * currentLetter.w), (gWindow.getHeight() / 7) + ((LEADERBOARD_LINES + 1) * (gWindow.getHeight() / 10)), NULL, &currentLetter);
                    }
                    lEnterNewNamePrompt.render(textLine.x, textLine.y + ((LEADERBOARD_LINES + 1) * textLine.h), NULL, &textLine);
                    SDL_RenderFillRect(gWindow.getRenderer(), &currentLetter);
                }else{
                    //name has been entered, we need to rerender the appropriate leaderboard entry
                    //build new name
                    leaderboardNames[index] = defaultName[0] + defaultName[1] + defaultName[2];
                    char header [5];
                    char line [10];
                    int rank = index + 1;
                    sprintf(header, "%d. ", rank);
                    sprintf(line, "%.3f", leaderboardScores[index]);
                    string entry = header + leaderboardNames[index] + " " + line;
                    gLeaderboardEntry[index].loadFromRenderedText(entry.c_str(), black);
                }
            }else{
                //users time didnt make the board but we want to show them still
                lUserTime.render(textLine.x, textLine.y + ((LEADERBOARD_LINES + 1) * textLine.h), NULL, &textLine);
            }
        }
        SDL_RenderPresent(gWindow.getRenderer());
    }
    SDL_StopTextInput();
    lEnterNewNamePrompt.free();
    lLeaderboardHeader.free();
    lPlayAgainPrompt.free();
    lUserTime.free();
    delayTimer.stop();
    return playAgain;
}

//method for the pregame setup, this is currently just rendering an instruction screen and getting a start
//signal from the user
void pregameSetup(bool* globalQuit){
    //flag to check if the user is ready
    //fade out music
    Mix_FadeOutMusic(500);
    bool userReady = false;
    SDL_Event e;
    //user key press prompt
    SDL_Color redText = {255,0,0};
    lTexture lUserPrompt = lTexture(gWindow.getRenderer());
    lUserPrompt.setFont(gFont);
    //need to set the text string here
    string promptString = "Press " + playerControls[3] + " and " + playerControls[6] + " to start!";
    if(!lUserPrompt.loadFromRenderedText(promptString.c_str(), redText)){
        printf("Could not load prompt string!\n");
    }
    //while waiting on the user display the instruction screen
    while(!userReady){
        while(SDL_PollEvent(&e) != 0){
            if(e.type == SDL_QUIT){
                userReady = true;
                *globalQuit = true;
            }
            //handle window event and UI resizing
            gWindow.handleEvent(e);
            resizeUI(&e);
            //check if user is ready, ready signal is the button for player 1 right and player 2 left held down
            if(e.type == SDL_KEYDOWN){
                //get the keystate
                const Uint8* keyState = NULL;
                keyState = SDL_GetKeyboardState(NULL);
                if(keyState[player1.getControlButton(3)] && keyState[player2.getControlButton(2)]){
                    Mix_PlayChannel(-1, gClickSound, 0);
                    userReady = true;
                }
            }
        }
        //in the meantime display the pregame splash
        gWindow.render();
        SDL_Rect screen = {0,0,gWindow.getWidth(), gWindow.getHeight()};
        gImageTextures[gPregameSplash]->render(0, 0, NULL, &screen);
        //render the prompt
        //scale the text up slightly
        SDL_Rect textBox = {0,0,(4*gWindow.getWidth()/5), (gWindow.getHeight()/8)};
        lUserPrompt.render((gWindow.getWidth() - textBox.w)/2, (3*(gWindow.getHeight()- textBox.h))/4, NULL, &textBox);
        SDL_RenderPresent(gWindow.getRenderer());
    }
    //get rid of user prompt
    lUserPrompt.free();
    //start the music
    Mix_VolumeMusic(80);
    Mix_PlayMusic(gGameMusic, -1);
    Mix_SetMusicPosition(2.9);
}
void loadLevel(int levelNumber){
    //set the level and starting positions
    player1.setLevelSize(LEVEL_WIDTH[levelNumber], LEVEL_HEIGHT[levelNumber]);
    player1.setStartingPos(0, 0);
    player1.setLevel(levelNumber);
    player2.setLevelSize(LEVEL_WIDTH[levelNumber], LEVEL_HEIGHT[levelNumber]);
    player2.setStartingPos(LEVEL_WIDTH[levelNumber], LEVEL_HEIGHT[levelNumber]);
    player2.setLevel(levelNumber);
    //reset the players
    player1.reset();
    player2.reset();
}

bool playLevel(bool* globalQuit, float stageTime, float currentRunTime){
    //need to load the stage time, run time, and play again prompts
    lTexture lStageTime = lTexture(gWindow.getRenderer());
    lStageTime.setFont(gFont);
    char sT [10];
    sprintf(sT, "%.3f", stageTime);
    string stageTimeLine = "Stage completed in: ";
    stageTimeLine += sT;
    if(!lStageTime.loadFromRenderedText(stageTimeLine, black)){
        printf("Could not load stage time text!\n");
    }
    lTexture lRunTime = lTexture(gWindow.getRenderer());
    lRunTime.setFont(gFont);
    char rT [10];
    sprintf(rT, "%.3f", currentRunTime);
    string lRunTimeLine = "Current run time: ";
    lRunTimeLine += rT;
    if(!lRunTime.loadFromRenderedText(lRunTimeLine, black)){
        printf("Could not load run time text!\n");
    }
    lTexture lNextLevelPrompt = lTexture(gWindow.getRenderer());
    lNextLevelPrompt.setFont(gFont);
    string nextLevelPrompt = "Play next level? (y/n)";
    if(!lNextLevelPrompt.loadFromRenderedText(nextLevelPrompt, black)){
        printf("Could not load next level prompt text!\n");
    }
    bool playNextLevel = true;
    bool selectionMade = false;
    SDL_Event e;
    while(!selectionMade){
        while(SDL_PollEvent(&e) != 0){
            if(e.type == SDL_QUIT){
                //user wants to quit
                selectionMade = true;
                playNextLevel = false;
                *globalQuit = true;
            }
            gWindow.handleEvent(e);
            //handle UI resizing
            resizeUI(&e);
            if(e.type == SDL_KEYDOWN){
                //check if user wants to play next level
                switch (e.key.keysym.sym) {
                    case SDLK_y:
                        selectionMade = true;
                        playNextLevel = true;
                        break;
                    case SDLK_n:
                        selectionMade = true;
                        playNextLevel = false;
                    default:
                        break;
                }
            }
        }
        //render here
        SDL_Rect screen = {0,0,gWindow.getWidth(), gWindow.getHeight()};
        gWindow.render();
        SDL_Rect textLine = {(gWindow.getWidth() - (8 * (gWindow.getWidth() / 10))) / 2, (gWindow.getHeight() - (gWindow.getHeight() / 10))/ 2, 8 * (gWindow.getWidth() / 10), gWindow.getHeight() / 10};
        gImageTextures[gNextLevelSplash]->render(0, 0, NULL, &screen);
        //render prompts
        lStageTime.render(textLine.x, textLine.y, NULL, &textLine);
        lRunTime.render(textLine.x, textLine.y + textLine.h, NULL, &textLine);
        lNextLevelPrompt.render(textLine.x, gWindow.getHeight() - textLine.h, NULL, &textLine);
        SDL_RenderPresent(gWindow.getRenderer());
    }
    //free textures
    lStageTime.free();
    lRunTime.free();
    lNextLevelPrompt.free();
    return playNextLevel;
}
#endif /* lGameMethods_h */
