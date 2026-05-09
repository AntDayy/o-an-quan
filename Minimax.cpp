#include <bits/stdc++.h>

using namespace std;

// ========================================================
// 1. LỚP MOVE (Nước đi)
// ========================================================
class Move
{
public:
    int cell;      // Ô được chọn để đi
    int direction; // Hướng đi: 1 (Phải), -1 (Trái)

    // LƯU Ý: Ở bản Minimax này, biến winRate không còn là % tỷ lệ thắng nữa.
    // Nó được dùng để lưu "Điểm thế cờ" (Evaluation score) của nước đi đó.
    // Điểm càng cao nghĩa là thế cờ càng có lợi cho mình.
    double winRate;

    Move(int c, int d) : cell(c), direction(d), winRate(0.0) {}
};

// ========================================================
// 2. LỚP GAMESTATE (Trạng thái bàn cờ)
// Quản lý luật chơi, y hệt như bản Monte Carlo nhưng có thêm hàm tính điểm
// ========================================================
class GameState
{
private:
    vector<int> board;
    int score[2];
    int currentTurn;

    // Hàm rải bù quân nếu 5 ô dân đều trống
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

    // [MỚI] Hàm tính TỔNG TÀI SẢN của Người chơi 1 (P1)
    // Bằng: Điểm đã ăn được + Số quân còn đang nằm trên 5 ô dân của P1
    int getP1Total() const
    {
        int total = score[0];
        for (int i = 1; i <= 5; i++)
            total += board[i];
        return total;
    }

    // [MỚI] Hàm tính TỔNG TÀI SẢN của Người chơi 2 (P2)
    int getP2Total() const
    {
        int total = score[1];
        for (int i = 7; i <= 11; i++)
            total += board[i];
        return total;
    }

    // Lấy các nước đi hợp lệ
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

    // Thực hiện nước đi (rải quân, ăn quân)
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

// ========================================================
// 3. LỚP MINIMAX AI (Trí tuệ nhân tạo duyệt cây)
// ========================================================
class MinimaxAI
{
private:
    int maxDepth; // Độ sâu tối đa mà AI sẽ "nhìn trước" (ví dụ: nhìn trước 7 nước đi)

    // Hàm Lượng Giá (Heuristic): Đánh giá xem bàn cờ hiện tại ai đang có lợi hơn.
    int evaluate(GameState state, int myTurn)
    {
        if (state.isGameOver())
        {
            state.finalizeGame(); // Thu dọn tàn cuộc nếu game hết
        }
        int p1Score = state.getP1Total(); // Tổng tài sản P1
        int p2Score = state.getP2Total(); // Tổng tài sản P2

        // Nếu AI là P1 (0): Điểm = Tài sản P1 - Tài sản P2
        // Nếu AI là P2 (1): Điểm = Tài sản P2 - Tài sản P1
        // Điểm > 0: AI đang thắng thế. Điểm < 0: AI đang thua thiệt.
        return (myTurn == 0) ? (p1Score - p2Score) : (p2Score - p1Score);
    }

    // Thuật toán Minimax cốt lõi kết hợp cắt tỉa Alpha-Beta
    // alpha: Điểm tốt nhất (cao nhất) mà AI chắc chắn có được cho đến nay (Max)
    // beta: Điểm tốt nhất (thấp nhất) mà Đối thủ chắc chắn ép được AI cho đến nay (Min)
    int minimax(GameState state, int depth, int alpha, int beta, bool isMaximizing, int myTurn)
    {
        // Điều kiện dừng đệ quy: Đạt đến độ sâu tối đa hoặc game đã kết thúc
        if (depth == 0 || state.isGameOver())
        {
            return evaluate(state, myTurn); // Tính điểm của bàn cờ tại nhánh này
        }

        vector<Move> moves = state.getValidMoves();
        // Nếu không có nước đi nào hợp lệ
        if (moves.empty())
        {
            return evaluate(state, myTurn);
        }

        // Lượt của AI (Cố gắng tìm nước đi làm tối đa hóa điểm số)
        if (isMaximizing)
        {
            int maxEval = -999999; // Khởi tạo điểm cực thấp
            for (const auto &m : moves)
            {
                GameState nextState = state;
                nextState.makeMove(m);
                // Gọi đệ quy cho lượt của Đối thủ (isMaximizing = false), độ sâu giảm 1
                int eval = minimax(nextState, depth - 1, alpha, beta, false, myTurn);

                maxEval = max(maxEval, eval); // Lấy điểm cao nhất trong các lựa chọn
                alpha = max(alpha, eval);     // Cập nhật giá trị alpha

                // CẮT TỈA ALPHA-BETA:
                // Nếu đối thủ (beta) có một lựa chọn tốt hơn ở một nhánh khác,
                // họ sẽ không bao giờ để chúng ta đi vào nhánh hiện tại. Dừng duyệt nhánh này luôn!
                if (beta <= alpha)
                    break;
            }
            return maxEval;
        }
        // Lượt của Đối thủ (Cố gắng tìm nước đi làm tối thiểu hóa điểm số của AI)
        else
        {
            int minEval = 999999; // Khởi tạo điểm cực cao
            for (const auto &m : moves)
            {
                GameState nextState = state;
                nextState.makeMove(m);
                // Gọi đệ quy cho lượt của AI (isMaximizing = true), độ sâu giảm 1
                int eval = minimax(nextState, depth - 1, alpha, beta, true, myTurn);

                minEval = min(minEval, eval); // Chọn điểm thấp nhất (gây thiệt hại nhất cho AI)
                beta = min(beta, eval);       // Cập nhật giá trị beta

                // CẮT TỈA: Giống như trên, nếu thấy đường này AI sẽ có lợi hơn alpha,
                // AI sẽ chọn alpha ở nhánh khác chứ không thèm đi nhánh này, nên đối thủ khỏi xét tiếp.
                if (beta <= alpha)
                    break;
            }
            return minEval;
        }
    }

    // Tính điểm phản công tốt nhất mà đối thủ đạt được sau khi mình đi xong
    // Trả về càng cao = đối thủ càng nguy hiểm cho mình → ta muốn số này nhỏ
    int opponentBestCounter(GameState stateAfterMove, int myTurn)
    {
        vector<Move> oppMoves = stateAfterMove.getValidMoves();
        if (oppMoves.empty())
            return 0;

        int worst = -999999;
        for (const auto &m : oppMoves)
        {
            GameState next = stateAfterMove;
            next.makeMove(m);

            // Đánh giá bàn cờ theo góc nhìn của đối thủ (ngược lại với myTurn)
            int oppGain = (myTurn == 0) ? (next.getP2Total() - next.getP1Total())
                                        : (next.getP1Total() - next.getP2Total());
            worst = max(worst, oppGain);
        }
        return worst; // Đối thủ có thể gây thiệt hại tối đa bao nhiêu
    }

public:
    // Khởi tạo AI với độ sâu nhìn trước là 7 bước
    MinimaxAI(int depth = 7) : maxDepth(depth) {}

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
            m.winRate = minimax(nextState, maxDepth - 1, -999999, 999999, false, myTurn);
        }

        // Tìm điểm cao nhất
        double bestScore = max_element(moves.begin(), moves.end(),
                                       [](const Move &a, const Move &b)
                                       { return a.winRate < b.winRate; })
                               ->winRate;

        // Tách riêng các nước đi có cùng điểm cao nhất
        vector<Move> topMoves, otherMoves;
        for (const auto &m : moves)
        {
            if (m.winRate == bestScore)
                topMoves.push_back(m);
            else
                otherMoves.push_back(m);
        }

        // Phá thế: trong topMoves, ưu tiên nước khiến đối thủ phản công kém nhất
        if (topMoves.size() > 1)
        {
            // Tính điểm phụ: đối thủ phản công tốt nhất được bao nhiêu sau mỗi nước đi
            vector<pair<int, int>> counterScores; // {chỉ số, điểm phản công của đối thủ}
            for (int i = 0; i < (int)topMoves.size(); i++)
            {
                GameState nextState = currentState;
                nextState.makeMove(topMoves[i]);
                int oppCounter = opponentBestCounter(nextState, myTurn);
                counterScores.push_back({i, oppCounter});
            }

            // Tìm mức phản công thấp nhất của đối thủ
            int minCounter = min_element(counterScores.begin(), counterScores.end(),
                                         [](const pair<int, int> &a, const pair<int, int> &b)
                                         {
                                             return a.second < b.second;
                                         })
                                 ->second;

            // Chỉ giữ lại các nước mà đối thủ phản công ở mức thấp nhất đó
            vector<Move> filtered;
            for (auto &[idx, score] : counterScores)
            {
                if (score == minCounter)
                    filtered.push_back(topMoves[idx]);
            }

            // Fallback: nếu vẫn còn nhiều nước bằng nhau → chọn ngẫu nhiên
            if (filtered.size() > 1)
            {
                int randIdx = rand() % filtered.size();
                swap(filtered[0], filtered[randIdx]);
            }

            topMoves = filtered;
        }

        // Ghép lại: nước tốt nhất lên đầu, các nước còn lại sắp theo điểm
        sort(otherMoves.begin(), otherMoves.end(),
             [](const Move &a, const Move &b)
             { return a.winRate > b.winRate; });

        vector<Move> result;
        result.insert(result.end(), topMoves.begin(), topMoves.end());
        result.insert(result.end(), otherMoves.begin(), otherMoves.end());
        return result;
    }
};

// ========================================================
// 4. HÀM MAIN (Tương tác nhập xuất)
// ========================================================
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

        // Khởi tạo Minimax AI với độ sâu 7 (nhìn xa 7 lượt luân phiên)
        // Lưu ý: Độ sâu càng cao chạy càng chậm nhưng càng thông minh.
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
            cout << "---------------------------------------------" << endl;
            // Đổi nhãn "Ti le thang" thành "Diem the co" cho đúng bản chất Minimax
            cout << setw(10) << "O di" << setw(15) << "Huong" << setw(20) << "Diem the co" << endl;

            for (const Move &r : results)
            {
                string dirStr = (r.direction == 1) ? "Phai" : "Trai";
                cout << setw(10) << r.cell << setw(15) << dirStr
                     << setw(18) << r.winRate << endl; // winRate lúc này là Điểm (ví dụ: +5, -2, 0)
            }
            cout << "---------------------------------------------" << endl;
            cout << "=> Loi khuyen an toan nhat: Nen di o " << results[0].cell
                 << " huong " << ((results[0].direction == 1) ? "phai" : "trai") << endl;
        }
    }

    return 0;
}