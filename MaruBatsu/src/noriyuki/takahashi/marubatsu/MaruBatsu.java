/*
 * Copyright 2011 Noriyuki Takahashi
 *
 * A simple maru-batsu game example for studying how to use TextView with click events.  Once a game
 * is over, the user needs to restart the application to start a new game by pushing the back button
 * of Android.
 */

package noriyuki.takahashi.marubatsu;

import android.app.Activity;
import android.os.Bundle;
import android.view.View.OnClickListener;
import android.view.View;
import android.widget.TextView;

class MaruBatsuGame {
  private static final int CELL_SIZE = 3;
  public enum Player { NONE, MARU, BATSU }
  public enum GameState { ONGOING, MARU_WIN, BATSU_WIN, DRAW };
  
  public MaruBatsuGame() {
    mCells = new Player[CELL_SIZE][CELL_SIZE];
    for (int i = 0; i < CELL_SIZE; ++i) {
      for (int j = 0; j < CELL_SIZE; ++j) {
        mCells[i][j] = Player.NONE;
      }
    }
    mCurrentPlayer = Player.MARU;
    mGameState = GameState.ONGOING;
    mNumRemainingCells = CELL_SIZE * CELL_SIZE;
  }

  private Player getPlayerCompletingRow(int i) {
    if (mCells[i][0] == Player.NONE) {
      return Player.NONE;
    }
    for (int j = 1; j < CELL_SIZE; ++j) {
      if (mCells[i][j] != mCells[i][0]) {
        return Player.NONE;
      }
    }
    return mCells[i][0];
  }

  private Player getPlayerCompletingColumn(int j) {
    if (mCells[0][j] == Player.NONE) {
      return Player.NONE;
    }
    for (int i = 1; i < CELL_SIZE; ++i) {
      if (mCells[i][j] != mCells[0][j]) {
        return Player.NONE;
      }
    }
    return mCells[0][j];
  }

  private Player getPlayerCompletingRightDownDiagonal() {
    if (mCells[0][0] == Player.NONE) {
      return Player.NONE;
    }
    for (int k = 1; k < CELL_SIZE; ++k) {
      if (mCells[k][k] != mCells[0][0]) {
        return Player.NONE;
      }
    }
    return mCells[0][0];
  }

  private Player getPlayerCompletingRightUpDiagonal() {
    if (mCells[CELL_SIZE - 1][0] == Player.NONE) {
      return Player.NONE;
    }
    for (int k = 1; k < CELL_SIZE; ++k) {
      if (mCells[CELL_SIZE - 1 - k][k] != mCells[CELL_SIZE - 1][0]) {
        return Player.NONE;
      }
    }
    return mCells[CELL_SIZE - 1][0];
  }

  public boolean put(int i, int j) {
    if (mGameState != GameState.ONGOING || mCells[i][j] != Player.NONE) {
      return false;
    }
    mCells[i][j] = mCurrentPlayer;
    mCurrentPlayer = (mCurrentPlayer == Player.MARU) ? Player.BATSU : Player.MARU;
    --mNumRemainingCells;

    Player winner;
    if ((winner = getPlayerCompletingRow(i)) != Player.NONE ||
        (winner = getPlayerCompletingColumn(j)) != Player.NONE ||
        (winner = getPlayerCompletingRightDownDiagonal()) != Player.NONE ||
        (winner = getPlayerCompletingRightUpDiagonal()) != Player.NONE) {
      mGameState = (winner == Player.MARU) ? GameState.MARU_WIN : GameState.BATSU_WIN;
    } else if (mNumRemainingCells == 0) {
      mGameState = GameState.DRAW;
    }
    
    return true;
  }

  public Player getCell(int i, int j) {
    return mCells[i][j];
  }

  public Player getCurrentPlayer() {
    return mCurrentPlayer;
  }

  public GameState getGameState() {
    return mGameState;
  }
  
  private Player[][] mCells;
  private Player mCurrentPlayer;
  private GameState mGameState;
  private int mNumRemainingCells;
}

public class MaruBatsu extends Activity {
  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.main);

    mGame = new MaruBatsuGame();

    // This pattern is not nice... but it's OK for example.
    ((TextView) findViewById(R.id.cell0)).setOnClickListener(new OnCellClickListener(0, 0));
    ((TextView) findViewById(R.id.cell1)).setOnClickListener(new OnCellClickListener(0, 1));
    ((TextView) findViewById(R.id.cell2)).setOnClickListener(new OnCellClickListener(0, 2));
    ((TextView) findViewById(R.id.cell3)).setOnClickListener(new OnCellClickListener(1, 0));
    ((TextView) findViewById(R.id.cell4)).setOnClickListener(new OnCellClickListener(1, 1));
    ((TextView) findViewById(R.id.cell5)).setOnClickListener(new OnCellClickListener(1, 2));
    ((TextView) findViewById(R.id.cell6)).setOnClickListener(new OnCellClickListener(2, 0));
    ((TextView) findViewById(R.id.cell7)).setOnClickListener(new OnCellClickListener(2, 1));
    ((TextView) findViewById(R.id.cell8)).setOnClickListener(new OnCellClickListener(2, 2));
  }

  private void setMessage(String msg) {
    ((TextView) findViewById(R.id.message)).setText(msg);
  }
  
  private class OnCellClickListener implements View.OnClickListener {
    private final int mRow;
    private final int mColumn;
    
    public OnCellClickListener(int i, int j) {
      mRow = i;
      mColumn = j;
    }
    
    @Override
    public void onClick(View v) {
      if (!mGame.put(mRow, mColumn)) {
        return;
      }

      switch (mGame.getCell(mRow, mColumn)) {
        case MARU:
          ((TextView) v).setText("O");
          break;
        case BATSU:
          ((TextView) v).setText("X");
          break;
      }

      switch (mGame.getGameState()) {
        case ONGOING:
          if (mGame.getCurrentPlayer() == MaruBatsuGame.Player.MARU) {
            setMessage("O's turn");
          } else {
            setMessage("X's turn");
          }
          break;
        case MARU_WIN:
          setMessage("O win!");
          break;
        case BATSU_WIN:
          setMessage("X win!");
          break;
        case DRAW:
          setMessage("Draw :(");
          break;          
      }
    }
  }

  private MaruBatsuGame mGame;  
}
