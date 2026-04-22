#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <string>
#include <sstream>
#include <bits/stdc++.h>

using namespace std;

// ==========================================
// LỚP MOVE: Đại diện cho 1 nước đi
// ==========================================
class Move
{
public:
    int cell;
    int direction;  // 1: Phải (CW), -1: Trái (CCW)
    double winRate; // Ở Minimax, biến này dùng để lưu điểm số đánh giá (Evaluation Score)

    Move(int c, int d) : cell(c), direction(d), winRate(0.0) {}
};

// ==========================================
// LỚP GAMESTATE: Quản lý trạng thái và luật chơi
// ==========================================
class GameState
{
private:
    vector<int> board;
    int score[2];
    int currentTurn;

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
    GameState(const vector<int> &initialBoard, int p1Score, int p2Score, int turn)
    {
        board = initialBoard;
        score[0] = p1Score;
        score[1] = p2Score;
        currentTurn = turn;
    }

    GameState(const GameState &other)
    {
        board = other.board;
        score[0] = other.score[0];
        score[1] = other.score[1];
        currentTurn = other.currentTurn;
    }

    bool isGameOver() const
    {
        return (board[0] == 0 && board[6] == 0);
    }

    int getTurn() const
    {
        return currentTurn;
    }

    // Lấy tổng tài sản của P1 (Điểm đã ăn + Quân đang có trên sân nhà)
    int getP1Total() const
    {
        int total = score[0];
        for (int i = 1; i <= 5; i++)
            total += board[i];
        return total;
    }

    // Lấy tổng tài sản của P2
    int getP2Total() const
    {
        int total = score[1];
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
};

// ==========================================
// LỚP MINIMAXAI: AI sử dụng Minimax + Alpha-Beta
// ==========================================
class MinimaxAI
{
private:
    int maxDepth;

    // Hàm đánh giá thế cờ (Heuristic)
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

    // Thuật toán duyệt cây Minimax
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
                    break; // Cắt tỉa Alpha
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
                    break; // Cắt tỉa Beta
            }
            return minEval;
        }
    }

public:
    MinimaxAI(int depth = 7) : maxDepth(depth) {} // Khuyến nghị depth từ 6 đến 9

    vector<Move> findBestMoves(GameState currentState)
    {
        vector<Move> moves = currentState.getValidMoves();
        if (moves.empty())
            return moves;

        int myTurn = currentState.getTurn();

        for (auto &m : moves)
        {
            GameState nextState = currentState;
            nextState.makeMove(m);
            // Sau nước đi của mình, sẽ đến lượt đối thủ (isMaximizing = false)
            int moveValue = minimax(nextState, maxDepth - 1, -999999, 999999, false, myTurn);
            m.winRate = moveValue; // Lưu điểm vào winRate để tái sử dụng cấu trúc cũ
        }

        // Sắp xếp các nước đi theo điểm đánh giá giảm dần
        sort(moves.begin(), moves.end(), [](const Move &a, const Move &b)
             { return a.winRate > b.winRate; });

        return moves;
    }
};

// ==========================================
// HÀM MAIN: Giao diện vòng lặp liên tục
// ==========================================
int main()
{
    while (true)
    {
        cout << "\n==================================================" << endl;
        cout << "Nhap trang thai ban co (12 so) hoac 'end':\n> ";

        string inputStr;
        getline(cin, inputStr);

        if (inputStr == "end" || inputStr == "End" || inputStr == "END")
        {
            break;
        }

        stringstream ss(inputStr);
        vector<int> board(12);
        bool isValidInput = true;
        for (int i = 0; i < 12; i++)
        {
            if (!(ss >> board[i]))
            {
                isValidInput = false;
                break;
            }
        }

        if (!isValidInput)
        {
            cout << "Loi: Vui long nhap dung 12 so hoac 'end'!" << endl;
            continue;
        }

        int turn;
        cout << "Luot di hien tai (1 cho P1, 2 cho P2): ";
        cin >> turn;

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        turn -= 1;

        GameState game(board, 0, 0, turn);

        // Khởi tạo Minimax với độ sâu 7
        MinimaxAI ai(7);

        cout << "\nDang suy nghi bang Minimax Alpha-Beta...\n"
             << endl;
        vector<Move> results = ai.findBestMoves(game);

        if (results.empty())
        {
            cout << "Khong co nuoc di hop le!" << endl;
        }
        else
        {
            cout << "KET QUA TOI UU:" << endl;
            cout << "------------------------------------------" << endl;
            cout << setw(10) << "O di" << setw(15) << "Huong" << setw(20) << "Diem the co" << endl;

            for (const Move &r : results)
            {
                string dirStr = (r.direction == 1) ? "Phai" : "Trai";
                cout << setw(10) << r.cell << setw(15) << dirStr
                     << setw(18) << r.winRate << endl;
            }
            cout << "------------------------------------------" << endl;
            cout << "=> Loi khuyen an toan nhat: Nen di o " << results[0].cell
                 << " huong " << ((results[0].direction == 1) ? "Phai" : "Trai") << endl;
        }
    }

    return 0;
}