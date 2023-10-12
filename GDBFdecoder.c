#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// マクロ定数定義
// K:情報記号数
#define K 504
// N:符号長
#define N 1008
// ROW:検査行列内の各行の非零成分個数の最大値
#define ROW 6
// Line:検査行列内の各行の非零成分個数
#define Line 3

// 配列においてint型の変数n個分の領域確保
int *get_i_vector(int n)
{
    return (int *)malloc(sizeof(int) * n);
}

// テキストファイル"row1008.txt"からint型配列row_indexにコピー
void set_row_index(int row_index[ROW * (N - K)])
{
    FILE *fp;
    fp = fopen("row1008.txt", "r");
    for (int i = 0; i < N - K; i++)
    {
        for (int j = 0; j < ROW; j++)
        {
            fscanf(fp, "%d\n", &row_index[i * ROW + j]);
            // printf("%d ",row_index[i*ROW+j]);
        }
        // printf("\n");
    }
    fclose(fp);
}

// 送信符号語(codeword)の各成分を全て0(1)にする
void set_codeword(int codeword[N])
{
    int i;

    for (int i = 0; i < N; i++)
    {
        codeword[i] = 1;
    }
}

// int型配列codewordの成分表示
void print_codeword(int codeword[N])
{
    printf("codeword:\n");

    for (int i = 0; i < N; i++)
    {
        printf("%d ", codeword[i]);
    }

    printf("\n");
    printf("------------------\n");
}

// 引数snr:二元対称通信路における反転確率pに対する受信ベクトルを配列receiveに格納
void bsc(int codeword[N], int receive[N], double p)
{
    double y;
    
    for (int i = 0; i < N; i++)
    {
        y = (double)rand() / ((double)RAND_MAX + 1);

        if (y <= p)
        {
            if (codeword[i] == 1)
            {
                receive[i] = -1;
            }
            else
            {
                receive[i] = 1;
            }
        }
        else
        {
            receive[i] = codeword[i];
        }
    }
}

// int型配列receiveとcodewordの成分の誤り個数
int count_error(int receive[N], int codeword[N])
{
    int count = 0;
    for (int i = 0; i < N; i++)
    {
        if (receive[i] != codeword[i])
        {
            count++;
        }
    }
    return count;
}

// パリティシンボル計算
void check_node(int spc[ROW])
{
    int tmp[ROW];
    int s = 0;
    for (int i = 0; i < ROW; i++)
    {
        tmp[i] = spc[i];
    }

    // 検査行列内を参照　(シンドロームの計算)
    for (int i = 0; i < ROW; i++)
    {
        for (int j = 0; j < ROW; j++)
        {
            if (i == j)
            {
                ;
            }
            else
            {
                if (tmp[j] == -1)    
                {
                    s++;
                }
            }
        }
        if (s % 2 == 0) 
        {
            spc[i] = 1;
        }
        else
        {
            spc[i] = -1;
        }
        s = 0;
    }

    /*for( int i = 0; i < ROW; i++ )
    {
        if( tmp[i] == -1 )
        {
            s++;
        }
    }

    for( int i = 0; i < ROW; i++ )
    {
        if (s % 2 == 0) 
        {
            spc[i] = 1;
        }
        else
        {
            spc[i] = -1;
        }
    }*/
}

// main関数
int main(void)
{
    srand((unsigned int)time(NULL));
    // row_index:検査行列の各行における非零成分の列番号を格納
    int *row_index;
    // codeword:送信符号語
    int *codeword;
    // 反転確率p
    double p = 0.1;
    // receive:受信ベクトル
    int *receive;
    // estimate:推定ベクトル
    int *estimate;
    int *edge;
    int spc[ROW];
    int s = 0, s1 = 0, s2 = 0;
    int *sum;
    int erasure_dec = 0;
    // trial:試行回数
    int trial = 0;
    double wer_dec;
    // 反転確率pをファイル"p.txt"に書き込む
    // 各反転確率pにおけるWER(word error rate)をファイル"wer.txt"に書き込む
    FILE *fp1, *fp2;
    //fp1 = fopen("p.txt", "w");
    //fp2 = fopen("log(wer).txt", "w");

    // 領域確保
    row_index = get_i_vector(ROW * (N - K));
    codeword = get_i_vector(N);
    receive = get_i_vector(N);
    estimate = get_i_vector(N);
    edge = get_i_vector(ROW * (N - K));
    sum = get_i_vector(N);

    set_row_index(row_index);
    set_codeword(codeword);
    bsc(codeword, receive, p);
    // print_codeword(codeword);
    // print_codeword(receive);
    // printf("receive : %d\n",count_error(receive,codeword));

    for (;;)    //反転確率の無限ループ
    {
        for (;;)    //誤り確率が100に到達するまでの無限ループ
        {
            // trialをインクリメント
            trial++;
            // codewordとpから受信ベクトル(recieve)を初期化
            bsc(codeword, receive, p);

            //推定値の初期化
            for( int i = 0; i < N; i++ )
            {
                estimate[i] = receive[i];
                
                //printf( "estimate[%d]: %d\n", i, estimate[i] );
            } 

            for (int i = 0; i < (ROW * (N - K)); i++)   //非零成分全てへの参照
            {
                edge[i] = receive[row_index[i]];    //エッジにVNのシンボルの値を代入

                //printf( "edge: %d\n", edge[i]);
            }

            // 変数t:反復回数のカウント
            for (int t = 1; t < 301; t++)
            {
                for (int i = 0; i < N; i++)
                {
                    sum[i] = 0; //閾値の初期化
                }

                // パリティシンボル計算
                for (int j = 0;; j += ROW)  //各CNをみていく
                {
                    for (int i = 0; i < ROW; i++)   //CNと繋がっている各エッジの参照
                    {
                        spc[i] = edge[i + j];   //CNと繋がっているエッジの値をspcに格納
                    }

                    check_node(spc);    //シンドローム計算

                    for (int i = 0; i < ROW; i++)
                    {
                        edge[i + j] = spc[i];
                        //printf( "edge[%d]: %d\n", i+j, edge[i+j]);
                    }

                    if (s == (N - K - 1))   //全てCNを調べたら終わり
                    {
                        break;
                    }

                    s++;    //何番目のCNを参照しているかのカウント
                }
                s = 0;
                for (int i = 0; i < ROW * (N - K); i++) //全ての非零成分の数について
                {
                    sum[row_index[i]] += edge[i];   
                }

                for( int i = 0; i < N; i++ )
                {
                    sum[i] += receive[i]*estimate[i];
                }
                
                /*for(int i=0;i<N;i++){
                    printf("%d \n",sum[i]);
                }*/
                
                // 反転処理
                for (int i = 0; i < N; i++)
                {
                    if (sum[i] >= 0 )    
                    {
						estimate[i] = 1;
                    }
                    else    
                    {
						estimate[i] = -1;
                    }
                }

                for (int i = 0; i < (ROW * (N - K)); i++)
                {
                    edge[i] = estimate[row_index[i]];

                    //printf( "edge[%d]: %d\n", i, edge[i]);
                }
                // 誤り個数が0個であれば反復操作終了
                s1 = count_error(estimate, codeword);
                //printf("%d : %d\n",t,s1);
                if (s1 == 0)
                {
                    break;
                }
            }
            // 誤りが一つでもあればerrsure_decをインクリメント
            if (count_error(estimate, codeword) != 0)
            {
                erasure_dec++;
            }
            // erasure_decの値が100以上になれば試行終了
            if (erasure_dec == 100)
            {
                break;
            }
        }
        // word error rateを計算し、wer_decに計算結果を格納
        wer_dec = (double)erasure_dec / (double)trial;
        // printf("%d\n",erasure_dec);
        // 反転確率p,試行回数(trial), wer_est(wer)
        printf("p:%g trial:%d wer_dec:%f log10(wer_dec):%f\n", p, trial, wer_dec, log10(wer_dec));
        //fprintf(fp1, "%g,", p);
        //fprintf(fp2, "%g  %f\n", p, log(wer_dec));

        if (p >= 0.03)
        {
            p -= 0.01;
        }
        if (p < 0.03)
        {
            p -= 0.002;
        }
        if (p < 0.001)
        {
            break;
        }

        erasure_dec = 0;
        trial = 0;
    }
    // printf("%d",ROW/2);
    // print_codeword(estimate);

    // 領域解放
    free(row_index);
    free(codeword);
    free(receive);
    free(estimate);
    free(edge);
    free(sum);
    //fclose(fp1);
    //fclose(fp2);
}