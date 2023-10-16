#include "../evaluate.h"
#include "../position.h"

using namespace std;

namespace Eval
{
#ifndef EVAL_NO_USE

  int PieceValue[PIECE_NB] =
  {
    0, PawnValue, LanceValue, KnightValue, SilverValue, BishopValue, RookValue,GoldValue,
    KingValue, ProPawnValue, ProLanceValue, ProKnightValue, ProSilverValue, HorseValue, DragonValue,0,

    0, -PawnValue, -LanceValue, -KnightValue, -SilverValue, -BishopValue, -RookValue,-GoldValue,
    -KingValue, -ProPawnValue, -ProLanceValue, -ProKnightValue, -ProSilverValue, -HorseValue, -DragonValue,0,
  };

  int PieceValueCapture[PIECE_NB] =
  {
    VALUE_ZERO             , PawnValue * 2   , LanceValue * 2   , KnightValue * 2   , SilverValue * 2  ,
    BishopValue * 2, RookValue * 2, GoldValue * 2, KingValue , // SEE�Ŏg���̂ő傫�Ȓl�ɂ��Ă����B
    ProPawnValue + PawnValue, ProLanceValue + LanceValue, ProKnightValue + KnightValue, ProSilverValue + SilverValue,
    HorseValue + BishopValue, DragonValue + RookValue, VALUE_ZERO /* PRO_GOLD */,
    // KingValue�̒l�͎g��Ȃ�
    VALUE_ZERO             , PawnValue * 2   , LanceValue * 2   , KnightValue * 2   , SilverValue * 2  ,
    BishopValue * 2, RookValue * 2, GoldValue * 2, KingValue , // SEE�Ŏg���̂ő傫�Ȓl�ɂ��Ă����B
    ProPawnValue + PawnValue, ProLanceValue + LanceValue, ProKnightValue + KnightValue, ProSilverValue + SilverValue,
    HorseValue + BishopValue, DragonValue + RookValue, VALUE_ZERO /* PRO_GOLD */,
  };

  int ProDiffPieceValue[PIECE_NB] =
  {
    VALUE_ZERO, ProPawnValue - PawnValue, ProLanceValue - LanceValue, ProKnightValue - KnightValue, ProSilverValue - SilverValue, HorseValue - BishopValue, DragonValue - RookValue, VALUE_ZERO ,
    VALUE_ZERO, ProPawnValue - PawnValue, ProLanceValue - LanceValue, ProKnightValue - KnightValue, ProSilverValue - SilverValue, HorseValue - BishopValue, DragonValue - RookValue, VALUE_ZERO ,
    VALUE_ZERO, ProPawnValue - PawnValue, ProLanceValue - LanceValue, ProKnightValue - KnightValue, ProSilverValue - SilverValue, HorseValue - BishopValue, DragonValue - RookValue, VALUE_ZERO ,
    VALUE_ZERO, ProPawnValue - PawnValue, ProLanceValue - LanceValue, ProKnightValue - KnightValue, ProSilverValue - SilverValue, HorseValue - BishopValue, DragonValue - RookValue, VALUE_ZERO ,
  };

  ExtBonaPiece kpp_board_index[PIECE_NB] = {
    { BONA_PIECE_ZERO, BONA_PIECE_ZERO },
    { f_pawn, e_pawn },
    { f_lance, e_lance },
    { f_knight, e_knight },
    { f_silver, e_silver },
    { f_bishop, e_bishop },
    { f_rook, e_rook },
    { f_gold, e_gold },
    { f_king, e_king },
    { f_gold, e_gold }, // ����
    { f_gold, e_gold }, // ����
    { f_gold, e_gold }, // ���j
    { f_gold, e_gold }, // ����
    { f_horse, e_horse }, // �n
    { f_dragon, e_dragon }, // ��
    { BONA_PIECE_ZERO, BONA_PIECE_ZERO }, // ���̐���͂Ȃ�

                                          // ��肩�猩���ꍇ�Bf��e������ւ��B
    { BONA_PIECE_ZERO, BONA_PIECE_ZERO },
    { e_pawn, f_pawn },
    { e_lance, f_lance },
    { e_knight, f_knight },
    { e_silver, f_silver },
    { e_bishop, f_bishop },
    { e_rook, f_rook },
    { e_gold, f_gold },
    { e_king, f_king },
    { e_gold, f_gold }, // ����
    { e_gold, f_gold }, // ����
    { e_gold, f_gold }, // ���j
    { e_gold, f_gold }, // ����
    { e_horse, f_horse }, // �n
    { e_dragon, f_dragon }, // ��
    { BONA_PIECE_ZERO, BONA_PIECE_ZERO }, // ���̐���͂Ȃ�
  };

  ExtBonaPiece kpp_hand_index[COLOR_NB][KING] = {
    {
      { BONA_PIECE_ZERO, BONA_PIECE_ZERO },
      { f_hand_pawn, e_hand_pawn },
      { f_hand_lance, e_hand_lance },
      { f_hand_knight, e_hand_knight },
      { f_hand_silver, e_hand_silver },
      { f_hand_bishop, e_hand_bishop },
      { f_hand_rook, e_hand_rook },
      { f_hand_gold, e_hand_gold },
    },
    {
      { BONA_PIECE_ZERO, BONA_PIECE_ZERO },
      { e_hand_pawn, f_hand_pawn },
      { e_hand_lance, f_hand_lance },
      { e_hand_knight, f_hand_knight },
      { e_hand_silver, f_hand_silver },
      { e_hand_bishop, f_hand_bishop },
      { e_hand_rook, f_hand_rook },
      { e_hand_gold, f_hand_gold },
    },
  };

  // BonaPiece�̓��e��\������B���Ȃ�H,�Տ�̋�Ȃ珡�ځB��) HP3 (3���ڂ̎��̕�)
  std::ostream& operator<<(std::ostream& os, BonaPiece bp)
  {
    if (bp < fe_hand_end)
    {
      for (auto c : COLOR)
        for (Piece pc = PAWN; pc < KING; ++pc)
          if (kpp_hand_index[c][pc].fb <= bp && bp < kpp_hand_index[c][pc].fw)
          {
#ifdef PRETTY_JP
            os << "H" << pretty(pc) << int(bp - kpp_hand_index[c][pc].fb + 1); // ex.HP3
#else
            os << "H" << pc << int(bp - kpp_hand_index[c][pc].fb + 1); // ex.HP3
#endif
            break;
          }
    } else {
      for (auto pc : Piece())
        if (kpp_board_index[pc].fb <= bp && bp < kpp_board_index[pc].fb + SQ_NB)
        {
#ifdef PRETTY_JP
          os << Square(bp - kpp_board_index[pc].fb) << pretty(pc); // ex.32P
#else
          os << Square(bp - kpp_board_index[pc].fb) << pc; // ex.32P
#endif
          break;
        }
    }

    return os;
  }
#endif

}