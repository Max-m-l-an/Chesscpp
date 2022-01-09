#include "Headers/ui.h"
#include "Headers/Board.h"

void Ui::set_pieces(Piece *pl, const bool *pe) {
    own_pieces.clear();
    for (int piece = 0; piece < 64; piece++) {
        if (*(pe+piece)) {
            piece_exists[piece] = true;
            piece_list[piece] = *(pl+piece);
            if ((pl+piece)->get_color() == to_play) {
                own_pieces.push_back(piece);
            }
        } else {piece_exists[piece] = false;}

    }
}

tuple<bool, int, int, int> Ascii_ui::move() {
    bool got_move = false, try_pawn = false;
    tuple<bool, int, int, int> move;
    int old_pos, tar, promotion;
    string raw_move, raw_piece;
    while (!got_move) {
        old_pos = -1, tar = -1;

        if (!try_pawn) {
            promotion = -1;
            cin >> raw_move;
            replace(raw_move,"=", "");
            replace(raw_move,"x", "");
            replace(raw_move,"+", "");
            replace(raw_move,"#", "");}
        else {raw_move = "p"+raw_piece+raw_move;}

        if (raw_move.length() == 2 &&  raw_move != "uu") {raw_move = "p"+raw_move;}
        if (raw_move == "0-0" || raw_move == "O-O") {
            if (piece_exists[60-to_play*56] && piece_list[60-to_play*56].pos_in_real_moves(short(62-to_play*56))) {move = tuple(false, 60-to_play*56, 62-to_play*56, -1); return move;}
        } else if (raw_move  == "0-0-0" || raw_move == "O-O-O") {
            if (piece_exists[60-to_play*56] && piece_list[60-to_play*56].pos_in_real_moves(short (58-to_play*56))) {move = tuple(false, 60-to_play*56, 58-to_play*56, -1); return move;}
        }
        else if (raw_move == "u") {move = tuple(true, 1, 0, 0); return move;}
        else if (raw_move == "r") {move = tuple(true, 2, 0, 0); return move;}
        else if (raw_move == "uu") {move = tuple(true, 3, 0, 0); return move;}
        else if (raw_move == "to_last") {move = tuple(true, 4, 0, 0); return move;}
        else if (raw_move == "to start") {move = tuple(true, 5, 0, 0); return move;}
        else if (raw_move == "new game") {move = tuple(true, 6, 0, 0); return move;}
        else {
            int piece = convert_piece_type((raw_move.at(0))); // type of piece to move
            raw_piece = raw_move.at(0);
            raw_move = string(raw_move.begin()+1, raw_move.end());
            if (isalpha((char) raw_move.at(raw_move.length()-1))) {
                promotion = convert_piece_type(toupper(raw_move.at(raw_move.length()-1)));
                raw_move.pop_back();
            } // handles and removes promotions
            if ((raw_move.length() == 3 || raw_move.length() == 4) && !isalpha(raw_move.at(raw_move.length()-1))) {
                tar = (int(unsigned(raw_move.at(raw_move.length()-1)))-49)*8+int(unsigned(toupper(raw_move.at(raw_move.length()-2))))-65; // specific old_pos is given
                int row = -1, col = -1;
                if (isalpha(raw_move.at(0))) {
                    col = int (unsigned (toupper(raw_move.at(0)))) -65;
                    if (raw_move.length() == 4) {row = int (unsigned (raw_move.at(1)))-49;}
                } else {
                    row = int (unsigned (raw_move.at(0)))-49;
                }
                if (!(col == -1 || row == -1)) {
                    if (find(own_pieces.begin(), own_pieces.end(), row*8+col) != own_pieces.end() && piece_list[row*8+col].get_type() == piece) {old_pos = row*8+col;}
                } else {
                    for (int pos: own_pieces) {
                        if ((row == pos/8 || col == pos%8) && find(piece_list[pos].get_real_moves().begin(), piece_list[pos].get_real_moves().end(), tar) != piece_list[pos].get_real_moves().end() && piece_list[pos].get_type() == piece) {
                            old_pos = pos;
                            got_move = true;
                            break;
                        }
                    }
                }

            } else if (raw_move.length() == 2) {
                tar = (int(unsigned(raw_move.at(1)))-49)*8+int(unsigned(toupper(raw_move.at(0))))-65;
                for (int p = 0; p<64; p++) {
                    if (find(own_pieces.begin(), own_pieces.end(), p) == own_pieces.end()) {continue;}
                    vector<int> moves = piece_list[p].get_real_moves();
                    if (find(moves.begin(), moves.end(), tar) != moves.end() && piece_list[p].get_type() == piece) {
                        old_pos = p;
                        got_move = true;
                        break;
                    }
                }
            } else if (!try_pawn) {try_pawn = true; continue;}
            else {try_pawn = false; continue;}
        }
        if (old_pos == -1 || tar == -1) {try_pawn = true; continue;}
        move = tuple(false, old_pos, tar, promotion);
        return move;
    }
}

tuple<bool, int, int, int> Random_ui::move() {
    int piece;
    while (true) {
        piece = own_pieces.at(random()%own_pieces.size());
        if (!piece_list[piece].get_real_moves().empty()) {break;}
    }

    int move = piece_list[piece].get_real_moves().at(random()%piece_list[piece].get_real_moves().size());
    int promotion = -1;
    if (piece_list[piece].get_type() == 0 && ((to_play == 0 && (piece/8) == 1 && (move/8) == 0) || (to_play == 1 && (piece/8) == 6 && (move/8) == 7))){
        promotion = (int(random()))%4+1;
    }
    return {false, piece, move, promotion};
}
int count_undo = 0, count_moves =0;
tuple<bool, int, int, int> Bot_ui::move() {
    tuple<float, int, int, int> best_move;
    best_move = min_max(depth, to_play, -infinity(), infinity());
    cout << count_moves << endl;
    cout << game->get_fen();
    return {false, get<1>(best_move), get<2>(best_move), get<3>(best_move)};
}


tuple<float, int, int, int> Bot_ui::min_max(int local_depth, bool gameto_play, double alpha, double beta) {
    if (count_moves % 100 == 0) {cout << count_moves << endl;}
    double min_score = infinity(), max_score = -infinity();
    tuple<double, int,int, int> best_move;
    if (local_depth == 0) {return {evaluate(game->get_pl(), game->get_pe()), NULL, NULL, NULL};}
    // sort all possible moves by estimated (hardcoded) strength
    vector<tuple<int,int,int>> new_moves = order_moves(game->to_play);
    //go through all moves
    for (tuple<int,int,int> move:new_moves) {
        // move and update
        game->make_move(get<0>(move), get<1>(move), get<2>(move));
        int game_end = game->update_moves();
        // test if game over
        switch (game_end) {
            case -1:
                break;
            case 0:
                game->undo();
                return {(game->to_play ? -infinity()+depth-local_depth:infinity()-depth+local_depth), get<0>(move), get<1>(move), get<2>(move)};
            case 1:
                game->undo();
                return {(game->to_play ? infinity()-depth+local_depth:-infinity()+depth-local_depth), get<0>(move), get<1>(move), get<2>(move)};
            default:
                game->undo();
                return {0, get<0>(move), get<1>(move), get<2>(move)};
        }

        // prepare player specific stuff for next depth
        count_moves++;
        best_move = (game->to_play ? min_max(local_depth-1, false, alpha, min_score): min_max(local_depth-1, true, max_score, beta));
        game->undo();
        game->update_moves();
        count_undo++;
        double &score = get<0>(best_move);
        if (game->to_play == 1) {
            if (score > max_score) {
                max_score = score;
                if (local_depth == depth) {
                    best_move = {score, get<0>(move), get<1>(move), get<2>(move)};
                }
                if (max_score >= beta) {break;}
            } //else if (score > -infinity()) {best_move = {score, get<0>(move), get<1>(move), get<2>(move)};}
        } else {
            if (score < min_score) {
                min_score = score;
                if (local_depth == depth) {
                    best_move = {score, get<0>(move), get<1>(move), get<2>(move)};
                }
                if (min_score <= alpha) {break;}
            } //else if (score < infinity()) {best_move = {score, get<0>(move), get<1>(move), get<2>(move)};}
        }
    }
    if (local_depth == 2) {
        bool test = false;
    }
    if (game->to_play) {return {max_score, get<1>(best_move), get<2>(best_move), get<3>(best_move)};}
    else {return {min_score, get<1>(best_move), get<2>(best_move), get<3>(best_move)};}
}

float Bot_ui::evaluate(Piece *pl, const bool *pe) {
    int eval = 0;
    for (int i = 0; i <64; i++) {
        if (!(*(pe+i))) {continue;}
        Piece &piece = *(pl+i);
        eval += eval_list[piece.get_type()] * (-1+2*piece.get_color());
    }

    return (float) eval;
}
vector<tuple<int, int, int>> Bot_ui::order_moves(int color) {
    Piece *pl = game->get_pl();
    bool *pe = game->get_pe();
    vector<tuple<int, int, int>> ordered_moves;
    for (int i = 0; i < 64; i++) {
        if (!*(pe+i)) {continue;}
        Piece &piece = *(pl+i);
        if (color != piece.get_color()) {continue;}
        if (piece.get_type() == 0 && i == 1+5*color) {
            for (int move:piece.get_real_moves()) {
                for (int prom = 1; prom < 5; prom++) {
                    ordered_moves.emplace_back(i, move, prom);
                }
            }
        } else {
            for (int move:piece.get_real_moves()) {
                ordered_moves.emplace_back(i, move, -1);
            }
        }
    }
    return ordered_moves;
}
