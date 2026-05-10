#include <bits/stdc++.h>

using namespace std;

// ==========================================
// 1. LỚP NƯỚC ĐI (MOVE)
// ==========================================
class Move
{
public:
    int cell;
    int direction;
    double winRate;

    Move(int c, int d) : cell(c), direction(d), winRate(0.0) {}
};

// ==========================================
// 2. LỚP TRẠNG THÁI GAME (GAMESTATE)
// ==========================================
class GameState
{
private:
    vector<int> board;
    int score[2];
    int currentTurn;
    int quanValue[2]; // quanValue[0] = giá trị quân quan tại ô 0
                      // quanValue[1] = giá trị quân quan tại ô 6

    void replenishIfNeeded()
    {
        int start = (currentTurn == 0) ? 1 : 7;
        bool isEmpty = true;
        for (int i = start; i < start + 5; i++)
        {
            if (board[i] > 0)
            {
                isEmpty = false;
                break;
            }
        }

        if (isEmpty && !isGameOver())
        {
            for (int i = start; i < start + 5; i++)
            {
                board[i] = 1;
                score[currentTurn]--;
            }
        }
    }

public:
    // Constructor đầy đủ: nhận thêm quanValue cho ô 0 và ô 6
    GameState(const vector<int> &initialBoard, int p1Score, int p2Score, int turn,
              int qv0 = 0, int qv1 = 0)
    {
        board = initialBoard;
        score[0] = p1Score;
        score[1] = p2Score;
        currentTurn = turn;
        quanValue[0] = qv0;
        quanValue[1] = qv1;
    }

    GameState(const GameState &other)
    {
        board = other.board;
        score[0] = other.score[0];
        score[1] = other.score[1];
        currentTurn = other.currentTurn;
        quanValue[0] = other.quanValue[0];
        quanValue[1] = other.quanValue[1];
    }

    bool isGameOver() const
    {
        return (board[0] == 0 && board[6] == 0);
    }

    int getTurn() const
    {
        return currentTurn;
    }

    // Trả về số quân dân đang nằm trong ô quan (không tính bản thân quân quan)
    int getSmallPiecesInQuan(int quanPos) const
    {
        int idx = (quanPos == 0) ? 0 : 1;
        return max(0, board[quanPos] - quanValue[idx]);
    }

    // Trả về giá trị quân quan gốc tại ô (0 hoặc 6)
    int getQuanValue(int quanPos) const
    {
        return (quanPos == 0) ? quanValue[0] : quanValue[1];
    }

    int getP1Total() const
    {
        int total = score[0] * 5;
        for (int i = 1; i <= 5; i++)
            total += board[i];
        return total;
    }

    int getP2Total() const
    {
        int total = score[1] * 5;
        for (int i = 7; i <= 11; i++)
            total += board[i];
        return total;
    }

    vector<Move> getValidMoves()
    {
        replenishIfNeeded();
        vector<Move> moves;
        int start = (currentTurn == 0) ? 1 : 7;
        for (int i = start; i < start + 5; i++)
        {
            if (board[i] > 0)
            {
                moves.push_back(Move(i, 1));
                moves.push_back(Move(i, -1));
            }
        }
        return moves;
    }

    void makeMove(const Move &move)
    {
        int hand = board[move.cell];
        board[move.cell] = 0;
        int current = move.cell;
        int dir = move.direction;

        while (true)
        {
            while (hand > 0)
            {
                current = (current + dir + 12) % 12;
                board[current]++;
                hand--;
            }

            int next = (current + dir + 12) % 12;

            if (board[next] > 0 && next != 0 && next != 6)
            {
                hand = board[next];
                board[next] = 0;
                current = next;
            }
            else if (board[next] == 0)
            {
                while (board[next] == 0)
                {
                    int target = (next + dir + 12) % 12;
                    if (board[target] > 0)
                    {
                        score[currentTurn] += board[target];
                        board[target] = 0;
                        next = (target + dir + 12) % 12;
                        if (board[next] != 0)
                            break;
                    }
                    else
                    {
                        break;
                    }
                }
                break;
            }
            else
            {
                break;
            }
        }
        currentTurn = 1 - currentTurn;
    }

    void finalizeGame()
    {
        for (int i = 1; i <= 5; i++)
        {
            score[0] += board[i];
            board[i] = 0;
        }
        for (int i = 7; i <= 11; i++)
        {
            score[1] += board[i];
            board[i] = 0;
        }
    }

    int getWinner() const
    {
        if (score[0] > score[1])
            return 0;
        if (score[1] > score[0])
            return 1;
        return -1;
    }
};

// ==========================================
// 3. NGƯỜI CHƠI 1: MINIMAX (ALPHA-BETA)
// ==========================================
class MinimaxAI
{
private:
    int maxDepth;

    int evaluate(GameState state, int myTurn)
    {
        if (state.isGameOver())
        {
            state.finalizeGame();
        }
        int p1Score = state.getP1Total();
        int p2Score = state.getP2Total();

        return (myTurn == 0) ? (p1Score - p2Score) : (p2Score - p1Score);
    }

    int minimax(GameState state, int depth, int alpha, int beta, bool isMaximizing, int myTurn)
    {
        if (depth == 0 || state.isGameOver())
        {
            return evaluate(state, myTurn);
        }

        vector<Move> moves = state.getValidMoves();
        if (moves.empty())
        {
            return evaluate(state, myTurn);
        }

        if (isMaximizing)
        {
            int maxEval = -999999;
            for (const auto &m : moves)
            {
                GameState nextState = state;
                nextState.makeMove(m);
                int eval = minimax(nextState, depth - 1, alpha, beta, false, myTurn);

                maxEval = max(maxEval, eval);
                alpha = max(alpha, eval);

                if (beta <= alpha)
                    break;
            }
            return maxEval;
        }
        else
        {
            int minEval = 999999;
            for (const auto &m : moves)
            {
                GameState nextState = state;
                nextState.makeMove(m);
                int eval = minimax(nextState, depth - 1, alpha, beta, true, myTurn);

                minEval = min(minEval, eval);
                beta = min(beta, eval);

                if (beta <= alpha)
                    break;
            }
            return minEval;
        }
    }

public:
    MinimaxAI(int depth = 7) : maxDepth(depth) {}

    Move getBestMove(GameState currentState)
    {
        vector<Move> moves = currentState.getValidMoves();
        if (moves.empty())
            return Move(-1, 0);

        int myTurn = currentState.getTurn();
        Move bestMove = moves[0];
        int bestValue = -999999;

        for (auto &m : moves)
        {
            GameState nextState = currentState;
            nextState.makeMove(m);
            int moveValue = minimax(nextState, maxDepth - 1, -999999, 999999, false, myTurn);

            if (moveValue > bestValue)
            {
                bestValue = moveValue;
                bestMove = m;
                bestMove.winRate = bestValue;
            }
        }

        return bestMove;
    }
};

// ==========================================
// 4. NGƯỜI CHƠI 2: MONTE CARLO
// ==========================================
class MonteCarloAI
{
private:
    int numSimulations;
    int maxMovesPerGame;

    int simulateRandomGame(GameState state)
    {
        int movesCount = 0;
        while (!state.isGameOver() && movesCount < maxMovesPerGame)
        {
            vector<Move> validMoves = state.getValidMoves();
            if (validMoves.empty())
                break;

            int randomIndex = rand() % validMoves.size();
            state.makeMove(validMoves[randomIndex]);
            movesCount++;
        }

        state.finalizeGame();
        return state.getWinner();
    }

public:
    MonteCarloAI(int simulations = 10000, int maxMoves = 200)
    {
        numSimulations = simulations;
        maxMovesPerGame = maxMoves;
    }

    Move getBestMove(GameState currentState)
    {
        vector<Move> possibleMoves = currentState.getValidMoves();
        if (possibleMoves.empty())
            return Move(-1, 0);

        int originalTurn = currentState.getTurn();
        Move bestMove = possibleMoves[0];
        double bestWinRate = -1.0;

        for (Move &move : possibleMoves)
        {
            int wins = 0;
            for (int i = 0; i < numSimulations; i++)
            {
                GameState simulatedState = currentState;
                simulatedState.makeMove(move);

                int winner = simulateRandomGame(simulatedState);

                if (winner == originalTurn)
                {
                    wins++;
                }
            }

            double currentWinRate = (double)wins / numSimulations * 100.0;
            if (currentWinRate > bestWinRate)
            {
                bestWinRate = currentWinRate;
                bestMove = move;
                bestMove.winRate = bestWinRate;
            }
        }

        return bestMove;
    }
};

// ==========================================
// 5. HÀM TẠO TRẠNG THÁI NGẪU NHIÊN (CÂN BẰNG)
// ==========================================
// Đảm bảo cân bằng bằng cách:
//   - Chia đều quân dân giữa phía P1 (ô 1-5) và P2 (ô 7-11), chỉ lệch tối đa ±2
//   - Chia đều điểm đã ăn ±1
//   - Ô quan (0, 6) là trung lập nên không tính vào phần cân bằng
GameState generateRandomState(int smallPiecesOnBoard)
{
    vector<int> board(12, 0);
    int qv0 = 0, qv1 = 0;

    // --- Bước 1: Random 1 hoặc 2 ô quan ---
    int numQuan = (rand() % 2) + 1;
    vector<int> quanCells;
    if (numQuan == 2)
        quanCells = {0, 6};
    else
        quanCells = {(rand() % 2 == 0) ? 0 : 6};

    // Đặt quân quan + tối đa 2 quân dân ngẫu nhiên vào ô quan
    int smallUsedInQuan = 0;
    for (int pos : quanCells)
    {
        int quanVal = (rand() % 10) + 1;
        int maxExtra = smallPiecesOnBoard - smallUsedInQuan;
        int extraSmall = (maxExtra > 0) ? (rand() % (maxExtra + 1)) : 0;

        board[pos] = quanVal + extraSmall;
        smallUsedInQuan += extraSmall;

        if (pos == 0)
            qv0 = quanVal;
        else
            qv1 = quanVal;
    }

    // --- Bước 2: Chia CÂN BẰNG quân dân còn lại cho hai phía ---
    int remainingSmall = smallPiecesOnBoard - smallUsedInQuan;

    // Chia đều, chỉ cho lệch tối đa ±2 để giữ cân bằng
    int half = remainingSmall / 2;
    int maxVariance = min(2, half);
    int variance = (maxVariance > 0) ? (rand() % (2 * maxVariance + 1) - maxVariance) : 0;

    int p1Small = max(0, min(remainingSmall, half + variance));
    int p2Small = remainingSmall - p1Small;

    // Rải ngẫu nhiên p1Small quân vào ô 1-5 (phía P1)
    for (int i = 0; i < p1Small; i++)
        board[(rand() % 5) + 1]++;

    // Rải ngẫu nhiên p2Small quân vào ô 7-11 (phía P2)
    for (int i = 0; i < p2Small; i++)
        board[(rand() % 5) + 7]++;

    // --- Bước 3: Chia CÂN BẰNG điểm đã ăn ---
    int capturedSmall = 50 - smallPiecesOnBoard;
    int capturedQuan = 0;
    if (board[0] == 0)
        capturedQuan += 5;
    if (board[6] == 0)
        capturedQuan += 5;
    int totalCaptured = max(0, capturedSmall + capturedQuan);

    // Chia đều, phần dư (nếu có) random cho một bên
    int p1Score = totalCaptured / 2;
    int p2Score = totalCaptured / 2;
    if (totalCaptured % 2 == 1)
    {
        if (rand() % 2 == 0)
            p1Score++;
        else
            p2Score++;
    }

    int turn = rand() % 2;
    return GameState(board, p1Score, p2Score, turn, qv0, qv1);
}

// ==========================================
// 6. MAIN - ĐẤU TRƯỜNG AI
// ==========================================
int main()
{
    srand(time(0));

    int smallPiecesOnBoard;
    cout << "Nhap so luong quan nho (dan) dang nam tren ban (tu 1 den 50): ";
    cin >> smallPiecesOnBoard;

    if (smallPiecesOnBoard < 1 || smallPiecesOnBoard > 50)
    {
        cout << "So luong quan khong hop le!" << endl;
        return 0;
    }

    int totalGames = 100;
    cout << "\nKhoi tao Giai dau: " << totalGames << " van co." << endl;
    cout << "- Player 1: Minimax Alpha-Beta (Do sau = 5)" << endl;
    cout << "- Player 2: Monte Carlo (Mo phong 1000 lan/nuoc)" << endl;
    cout << "Dang chay mo phong, vui long doi (Co the mat nhieu thoi gian)...\n"
         << endl;

    int p1Wins = 0;
    int p2Wins = 0;
    int draws = 0;

    MinimaxAI ai_minimax(5);
    MonteCarloAI ai_montecarlo(1000);

    for (int i = 1; i <= totalGames; i++)
    {
        GameState state = generateRandomState(smallPiecesOnBoard);

        int turnLimit = 200;
        int currentTurnCount = 0;

        while (!state.isGameOver() && currentTurnCount < turnLimit)
        {
            vector<Move> validMoves = state.getValidMoves();
            if (validMoves.empty())
                break;

            Move bestMove(-1, 0);

            if (state.getTurn() == 0)
            {
                bestMove = ai_minimax.getBestMove(state);
            }
            else
            {
                bestMove = ai_montecarlo.getBestMove(state);
            }

            if (bestMove.cell != -1)
            {
                state.makeMove(bestMove);
            }
            else
            {
                break;
            }
            currentTurnCount++;
        }

        state.finalizeGame();
        int winner = state.getWinner();

        if (winner == 0)
            p1Wins++;
        else if (winner == 1)
            p2Wins++;
        else
            draws++;

        if (i % 10 == 0)
        {
            cout << "Da hoan thanh " << i << " / " << totalGames << " van..." << endl;
        }
    }

    cout << "\n==========================================" << endl;
    cout << "KET QUA DAU TRUONG AI (" << totalGames << " VAN)" << endl;
    cout << "==========================================" << endl;
    cout << "Minimax (P1) Thang    : " << p1Wins << " van (" << fixed << setprecision(2) << (double)p1Wins / totalGames * 100.0 << "%)" << endl;
    cout << "Monte Carlo (P2) Thang: " << p2Wins << " van (" << fixed << setprecision(2) << (double)p2Wins / totalGames * 100.0 << "%)" << endl;
    cout << "Hoa                   : " << draws << " van (" << fixed << setprecision(2) << (double)draws / totalGames * 100.0 << "%)" << endl;
    cout << "==========================================" << endl;

    return 0;
}