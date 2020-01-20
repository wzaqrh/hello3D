using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

public class BaseInfo
{
    public double r0, r1;
    public int[] burns = new int[3];

    public BaseInfo(double _r0, double _r1, int b0 = 0, int b1 = 0, int b2 = 0)
    {
        r0 = _r0;
        r1 = _r1;
        burns[0] = b0;
        burns[1] = b1;
        burns[2] = b2;
    }
}

public class Info
{
    public int LeftTurn;
    public int[] Burns = new int[3];

    public Info(int turn, int b0=0, int b1=0, int b2=0)
    {
        LeftTurn = turn;
        Burns[0] = b0;
        Burns[1] = b1;
        Burns[2] = b2;
    }

    public Info(Info other, int b0 = 0, int b1 = 0, int b2 = 0)
    {
        LeftTurn = other.LeftTurn;
        Burns[0] = Math.Max(other.Burns[0], b0);
        Burns[1] = Math.Max(other.Burns[1], b1);
        Burns[2] = Math.Max(other.Burns[2], b2);
    }
};

public class NewBehaviourScript : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        CalCMHM();
    }

    // Update is called once per frame
    void Update()
    {
        CalCMHM();
    }

    //CalCMHM
    void CalCMHM()
    {
        float f8_35 = FX(8, 0.35f);
        float f8_47 = FX(8, 0.47f);
        float z = f8_35;

        CalPreSkills();
    }

    float FX(int x, float n)
    {
        if (x <= 0) return 0.0f;
        return n + n * GX(x - 1, n) + (1 - n) * FX(x-1,n);
    }

    float GX(int x, float n)
    {
        if (x <= 0) return 0.0f;
        return 1 + n * GX(x - 1, n) + (1 - n) * FX(x - 1, n);
    }

    //CalXUXUN()
    void CalXUXUN()
    {
        double f8 = Fire0(new BaseInfo(0.35, 0.45, 3, 3, 3), new Info(8));
        double fbm = Fire0(new BaseInfo(0.47, 0.45, 3, 3, 3), new Info(8));
    }

    double Fire0(BaseInfo bctx, Info ctx)
    {
        double res = 0;
        if (ctx.LeftTurn > 0)
        {
            res += (1 - bctx.r0) * Fire1(bctx, new Info(ctx));//没点着
            res += 1 / 3 * bctx.r0 * Fire1(bctx, new Info(ctx, bctx.burns[0], 0, 0));//点到A
            res += 1 / 3 * bctx.r0 * Fire1(bctx, new Info(ctx, 0, bctx.burns[1], 0));//点到B
            res += 1 / 3 * bctx.r0 * Fire1(bctx, new Info(ctx, 0, 0, bctx.burns[2]));//点到C
        }
        return res;
    }

    double Fire1(BaseInfo bctx, Info ctx)
    {
        double res = 0;
        if (ctx.LeftTurn > 0)
        {
            res += (1 - bctx.r1) * Fire0(bctx, new Info(ctx));//没点着

            for (int i = 0; i < 3; ++i)
            {
                int[] arr = new int[3];
                arr[i] += bctx.burns[i];
                Info nextCtx = new Info(ctx, arr[0], arr[1], arr[2]);
                nextCtx.LeftTurn--;

                if (nextCtx.Burns[i] > 0)//爆炸
                {
                    res += 1 / 3 * bctx.r1 * 1;
                    nextCtx.Burns[i] = 0;
                    for (int j = 0; j < 3; ++j)
                    {
                        if (j != i && ctx.Burns[j] == 0)
                        {
                            nextCtx.Burns[j] = bctx.burns[i];
                        }
                    }
                }
                res += 1 / 3 * bctx.r1 * Fire0(bctx, nextCtx);
            }
        }
        return res;
    }

    //CalPrepare
    double CalNPH(double hurt, double rate)
    {
        return 8 * rate * hurt;
    }

    void CalPreSkills()
    {
        float huangzhong_rate = CalPrepare(8, 0.35f);
        float huangzhong_baimei_rate = CalPrepare(8, 0.47f);

        double hurts_a = (1.0f + huangzhong_rate / 8 / 4) * (huangzhong_rate * 1.8 * 3 + 8 + CalNPH(2.5, 0.35) + 7);
        double hurts_b = (1.0f + huangzhong_baimei_rate / 8 / 4) * (huangzhong_baimei_rate * 1.8 * 3 + 8 + CalNPH(2.5, 0.47));

        float a = 1.0f;
    }

    float CalPrepare(int x, float n)
    {
        float res = 0.0f;
        if (x >= 2)
        {
            res += n + n * CalPrepare(x - 2, n) + (1.0f - n) * CalPrepare(x-1,n);
        }
        return res;
    }
}
