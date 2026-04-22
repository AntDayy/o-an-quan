#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
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
    int direction; // 1: Phải (Cùng chiều kim đồng hồ), -1: Trái
    double winRate;

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
            // 1. Rải quân
            while (hand > 0)
            {
                current = (current + dir + 12) % 12;
                board[current]++;
                hand--;
            }

            // 2. Kiểm tra ô tiếp theo
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
// LỚP MONTECARLOAI: Trí tuệ nhân tạo
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

    vector<Move> findBestMoves(GameState currentState)
    {
        vector<Move> possibleMoves = currentState.getValidMoves();
        if (possibleMoves.empty())
            return possibleMoves;

        int originalTurn = currentState.getTurn();

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
            move.winRate = (double)wins / numSimulations * 100.0;
        }

        sort(possibleMoves.begin(), possibleMoves.end(), [](const Move &a, const Move &b)
             { return a.winRate > b.winRate; });

        return possibleMoves;
    }
};

// ==========================================
// HÀM MAIN: Giao diện vòng lặp liên tục
// ==========================================
int main()
{
    srand(time(0));

    while (true)
    {
        cout << "\n==================================================" << endl;
        cout << "Nhap trang thai ban co (12 so) hoac 'end':\n> ";

        string inputStr;
        getline(cin, inputStr);

        // Kiểm tra lệnh thoát
        if (inputStr == "end" || inputStr == "End" || inputStr == "END")
        {
            break;
        }

        // Dùng stringstream để bóc tách 12 số từ chuỗi nhập vào
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
            continue; // Quay lại đầu vòng lặp để nhập lại
        }

        int turn;
        cout << "Luot di hien tai (1 cho P1, 2 cho P2): ";
        cin >> turn;

        // Xóa bộ nhớ đệm (buffer) của cin để hàm getline() ở vòng lặp sau hoạt động đúng
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        turn -= 1; // Chuyển về 0 hoặc 1

        GameState game(board, 0, 0, turn);
        MonteCarloAI ai(10000);

        cout << "\nDang tinh toan mo phong...\n"
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
            cout << setw(10) << "O di" << setw(15) << "Huong" << setw(20) << "Ti le thang" << endl;

            for (const Move &r : results)
            {
                string dirStr = (r.direction == 1) ? "Phai" : "Trai ";
                cout << setw(10) << r.cell << setw(15) << dirStr
                     << setw(18) << fixed << setprecision(2) << r.winRate << "%" << endl;
            }
            cout << "------------------------------------------" << endl;
            cout << "=> Loi khuyen: Nen di o " << results[0].cell
                 << " huong " << ((results[0].direction == 1) ? "Phai" : "Trai") << endl;
        }
    }

    return 0;
}