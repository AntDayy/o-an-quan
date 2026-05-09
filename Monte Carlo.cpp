#include <bits/stdc++.h>

using namespace std;

// ========================================================
// 1. LỚP MOVE (Nước đi)
// Đại diện cho một hành động của người chơi
// ========================================================
class Move
{
public:
    int cell;       // Vị trí ô được chọn để bốc quân (từ 1-5 hoặc 7-11)
    int direction;  // Hướng rải quân: 1 là cùng chiều kim đồng hồ (Phải), -1 là ngược chiều (Trái)
    double winRate; // Tỷ lệ chiến thắng của nước đi này sau khi AI mô phỏng

    // Constructor khởi tạo nước đi ban đầu với tỷ lệ thắng = 0
    Move(int c, int d) : cell(c), direction(d), winRate(0.0) {}
};

// ========================================================
// 2. LỚP GAMESTATE (Trạng thái trò chơi)
// Quản lý bàn cờ, điểm số, lượt đi và các luật chơi Ô ăn quan
// ========================================================
class GameState
{
private:
    // Bàn cờ có 12 ô.
    // Ô 0 và 6 là 2 ô Quan.
    // Ô 1-5 là ô dân của Người chơi 1 (P1).
    // Ô 7-11 là ô dân của Người chơi 2 (P2).
    vector<int> board;
    int score[2];    // Điểm số: score[0] của P1, score[1] của P2
    int currentTurn; // Lượt đi hiện tại: 0 là P1, 1 là P2

    // Hàm rải thêm quân nếu đến lượt mà toàn bộ 5 ô dân của người đó đều trống (Luật mượn quân)
    void replenishIfNeeded()
    {
        // Xác định vị trí bắt đầu kiểm tra: P1 kiểm tra từ ô 1, P2 kiểm tra từ ô 7
        int start = (currentTurn == 0) ? 1 : 7;
        bool isEmpty = true;

        // Kiểm tra 5 ô của người chơi hiện tại xem có ô nào còn quân không
        for (int i = start; i < start + 5; i++)
        {
            if (board[i] > 0)
            {
                isEmpty = false;
                break; // Nếu có 1 ô có quân thì thoát, không cần rải bù
            }
        }

        // Nếu cả 5 ô đều trống và game chưa kết thúc
        if (isEmpty && !isGameOver())
        {
            // Trừ 5 điểm của người chơi (mượn 5 quân từ điểm đã ăn) để rải vào 5 ô
            for (int i = start; i < start + 5; i++)
            {
                board[i] = 1;         // Đặt 1 quân vào mỗi ô
                score[currentTurn]--; // Trừ 1 điểm cho mỗi ô (Tổng trừ 5 điểm)
            }
        }
    }

public:
    // Constructor khởi tạo trạng thái game
    GameState(const vector<int> &initialBoard, int p1Score, int p2Score, int turn)
    {
        board = initialBoard;
        score[0] = p1Score;
        score[1] = p2Score;
        currentTurn = turn;
    }

    // Copy Constructor (Tạo một bản sao của trạng thái game để AI mô phỏng mà không làm hỏng game thật)
    GameState(const GameState &other)
    {
        board = other.board;
        score[0] = other.score[0];
        score[1] = other.score[1];
        currentTurn = other.currentTurn;
    }

    // Kiểm tra game đã kết thúc chưa. Game kết thúc khi cả 2 ô Quan (0 và 6) đều không còn quân.
    bool isGameOver() const
    {
        return (board[0] == 0 && board[6] == 0);
    }

    // Lấy lượt đi hiện tại
    int getTurn() const
    {
        return currentTurn;
    }

    // Lấy danh sách tất cả các nước đi hợp lệ của người chơi hiện tại
    vector<Move> getValidMoves()
    {
        replenishIfNeeded(); // Kiểm tra và rải bù quân nếu cần trước khi lấy nước đi
        vector<Move> moves;
        int start = (currentTurn == 0) ? 1 : 7;

        // Duyệt qua 5 ô dân của mình, nếu ô nào có quân thì tạo ra 2 nước đi (rải trái & rải phải)
        for (int i = start; i < start + 5; i++)
        {
            if (board[i] > 0)
            {
                moves.push_back(Move(i, 1));  // Đi hướng phải
                moves.push_back(Move(i, -1)); // Đi hướng trái
            }
        }
        return moves;
    }

    // Hàm thực hiện một nước đi (Chứa logic luật chơi phức tạp nhất)
    void makeMove(const Move &move)
    {
        int hand = board[move.cell]; // Bốc toàn bộ quân ở ô được chọn lên tay
        board[move.cell] = 0;        // Ô vừa bốc trở thành 0
        int current = move.cell;     // Vị trí đang đứng
        int dir = move.direction;    // Hướng đi

        while (true)
        {
            // Bước 1: Rải quân trên tay
            while (hand > 0)
            {
                // Tính ô tiếp theo. Cộng thêm 12 rồi % 12 để tránh số âm quay vòng (Ví dụ: 0 - 1 = -1 -> 11)
                current = (current + dir + 12) % 12;
                board[current]++; // Bỏ 1 quân vào ô
                hand--;           // Trừ 1 quân trên tay
            }

            // Bước 2: Xem xét ô tiếp theo liền kề sau khi vừa rải hết quân
            int next = (current + dir + 12) % 12;

            // TH1: Ô tiếp theo có quân, và KHÔNG PHẢI LÀ Ô QUAN (0 và 6)
            if (board[next] > 0 && next != 0 && next != 6)
            {
                hand = board[next]; // Bốc tiếp số quân ở ô đó lên tay
                board[next] = 0;    // Ô đó trở thành 0
                current = next;     // Di chuyển vị trí hiện tại đến ô đó để tiếp tục vòng lặp rải
            }
            // TH2: Ô tiếp theo là ô trống (logic ăn quân)
            else if (board[next] == 0)
            {
                // Vòng lặp kiểm tra ăn liên hoàn (nhiều ô trống cách nhau)
                while (board[next] == 0)
                {
                    int target = (next + dir + 12) % 12; // Ô đích liền sau ô trống
                    // Nếu ô đích có quân -> Bị ăn
                    if (board[target] > 0)
                    {
                        score[currentTurn] += board[target]; // Cộng điểm cho người đang đi
                        board[target] = 0;                   // Lấy sạch quân ô đó

                        // Cập nhật 'next' để nhảy qua ô vừa ăn, tiếp tục xét xem có trống tiếp không
                        next = (target + dir + 12) % 12;

                        // Nếu ô nhảy tới tiếp theo lại có quân (hoặc là ô Quan) thì đứt mạch ăn liên hoàn, dừng lại
                        if (board[next] != 0)
                            break;
                    }
                    else
                    {
                        // Nếu ô đích liền sau ô trống lại là 1 ô trống khác -> Không ăn được, dừng lại
                        break;
                    }
                }
                break; // Kết thúc lượt
            }
            // TH3: Ô tiếp theo có quân NHƯNG LÀ Ô QUAN (hoặc đụng các luật cấm bốc tiếp khác)
            else
            {
                break; // Mất lượt
            }
        }
        currentTurn = 1 - currentTurn; // Đổi lượt đi: 0 thành 1, 1 thành 0
    }

    // Khi game kết thúc, dọn dẹp bàn cờ (ai làm chủ ô dân nào thì gom hết phần còn lại về điểm người đó)
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

    // Trả về người chiến thắng (0 là P1, 1 là P2, -1 là hòa)
    int getWinner() const
    {
        if (score[0] > score[1])
            return 0;
        if (score[1] > score[0])
            return 1;
        return -1;
    }
};

// ========================================================
// 3. LỚP MONTECARLO AI
// Trí tuệ nhân tạo dựa trên xác suất mô phỏng
// ========================================================
class MonteCarloAI
{
private:
    int numSimulations;  // Số lần chơi thử nghiệm (mặc định 10.000)
    int maxMovesPerGame; // Giới hạn số nước đi tối đa 1 ván để tránh kẹt lặp vô hạn

    // Hàm cho AI tự chơi 1 ván cờ bằng cách bốc ngẫu nhiên cho đến khi kết thúc
    int simulateRandomGame(GameState state)
    {
        int movesCount = 0;
        // Chơi đến khi hết game hoặc đạt số bước giới hạn
        while (!state.isGameOver() && movesCount < maxMovesPerGame)
        {
            vector<Move> validMoves = state.getValidMoves();
            if (validMoves.empty())
                break;

            // Chọn ngẫu nhiên 1 nước đi trong các nước hợp lệ
            int randomIndex = rand() % validMoves.size();
            state.makeMove(validMoves[randomIndex]);
            movesCount++;
        }

        state.finalizeGame();     // Thu dọn tàn cuộc
        return state.getWinner(); // Trả về xem ai thắng
    }

public:
    MonteCarloAI(int simulations = 10000, int maxMoves = 200)
    {
        numSimulations = simulations;
        maxMovesPerGame = maxMoves;
    }

    // Hàm cốt lõi: Tìm các nước đi tốt nhất cho trạng thái hiện tại
    vector<Move> findBestMoves(GameState currentState)
    {
        vector<Move> possibleMoves = currentState.getValidMoves();
        if (possibleMoves.empty())
            return possibleMoves;

        int originalTurn = currentState.getTurn(); // Ghi nhớ AI đang đóng vai trò P1 hay P2

        // Thử từng nước đi có thể có trên bàn cờ
        for (Move &move : possibleMoves)
        {
            int wins = 0; // Đếm số lần thắng nếu chọn nước đi này

            // Chơi thử numSimulations (10.000) ván cờ khác nhau với khởi đầu là nước đi này
            for (int i = 0; i < numSimulations; i++)
            {
                GameState simulatedState = currentState; // Tạo bản sao game
                simulatedState.makeMove(move);           // Thực hiện nước đi đang xét

                // Sau đó để 2 bên tự đi ngẫu nhiên đến hết ván
                int winner = simulateRandomGame(simulatedState);

                // Nếu kết quả AI thắng, tăng biến đếm
                if (winner == originalTurn)
                {
                    wins++;
                }
            }
            // Tính phần trăm chiến thắng của nước đi đó
            move.winRate = (double)wins / numSimulations * 100.0;
        }

        // Sắp xếp các nước đi theo tỷ lệ thắng từ Cao xuống Thấp
        sort(possibleMoves.begin(), possibleMoves.end(), [](const Move &a, const Move &b)
             { return a.winRate > b.winRate; });

        return possibleMoves;
    }
};

// ========================================================
// 4. HÀM MAIN
// Chạy chương trình tương tác với người dùng
// ========================================================
int main()
{
    srand(time(0)); // Cấp seed cho hàm random sinh số ngẫu nhiên theo thời gian thực

    while (true)
    {
        cout << "\n==================================================" << endl;
        cout << "Nhap trang thai ban co (12 so) hoac 'end':\n> ";

        string inputStr;
        getline(cin, inputStr);

        // Nhập "end" để thoát vòng lặp
        if (inputStr == "end" || inputStr == "End" || inputStr == "END")
        {
            break;
        }

        // Tách chuỗi nhập vào thành 12 số nguyên
        stringstream ss(inputStr);
        vector<int> board(12);
        bool isValidInput = true;
        for (int i = 0; i < 12; i++)
        {
            // Kiểm tra lỗi nếu nhập thiếu hoặc không phải là số
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

        // Xóa bộ nhớ đệm bàn phím sau lệnh cin để getline() lần sau không bị lỗi trôi lệnh
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        turn -= 1; // Đổi từ 1/2 sang 0/1 để khớp với logic mảng trong class

        // Khởi tạo GameState và MonteCarloAI
        GameState game(board, 0, 0, turn);
        MonteCarloAI ai(10000); // Mỗi nước đi sẽ mô phỏng 10,000 ván

        cout << "\nDang tinh toan mo phong...\n"
             << endl;

        // Gọi AI phân tích và trả về danh sách các nước đi đã xếp hạng
        vector<Move> results = ai.findBestMoves(game);

        if (results.empty())
        {
            cout << "Khong co nuoc di hop le!" << endl;
        }
        else
        {
            // In kết quả tư vấn ra màn hình một cách đẹp mắt
            cout << "KET QUA TOI UU:" << endl;
            cout << "---------------------------------------------" << endl;
            cout << setw(10) << "O di" << setw(15) << "Huong" << setw(20) << "Ti le thang" << endl;

            for (const Move &r : results)
            {
                string dirStr = (r.direction == 1) ? "Phai" : "Trai ";
                cout << setw(10) << r.cell << setw(15) << dirStr
                     << setw(18) << fixed << setprecision(2) << r.winRate << "%" << endl;
            }
            cout << "---------------------------------------------" << endl;
            // Gợi ý nước đi đầu tiên (vì đã được sort nên nó là tỷ lệ thắng cao nhất)
            cout << "=> Loi khuyen: Nen di o " << results[0].cell
                 << " huong " << ((results[0].direction == 1) ? "phai" : "trai") << endl;
        }
    }

    return 0;
}