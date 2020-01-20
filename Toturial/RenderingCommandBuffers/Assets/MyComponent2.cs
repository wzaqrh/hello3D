using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[AddComponentMenu("MyComponents/MyComponent2")]
public class MyComponent2 : MonoBehaviour
{
    [HideInInspector] public string Str1;
    public int Int1;

    [ContextMenu("ResetInt1")]
    void ResetInt1()
    {
        Int1 = 0;
    }
}