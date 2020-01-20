using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlaySound : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    void PlaySnd1(AnimationEvent evt)
    {
        switch(evt.intParameter)
        {
            case 0:
                Debug.Log("PlaySnd1 0");
                break;
            case 1:
                Debug.Log("PlaySnd1 1");
                break;
            default:
                break;
        }
    }

    void PlaySnd2(int intParameter)
    {
        switch (intParameter)
        {
            case 0:
                Debug.Log("PlaySnd1 0");
                break;
            case 1:
                Debug.Log("PlaySnd1 1");
                break;
            default:
                break;
        }
    }

    void EnterPause(int intParameter)
    {
        var animator = GetComponent<Animator>();
        animator.SetTrigger("Pause");
    }
}
