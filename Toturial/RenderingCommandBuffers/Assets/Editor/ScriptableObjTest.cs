using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

[System.Serializable]
public class MyField
{
    public int myInt = 10;
    public Color myColor = Color.white;
}

[CreateAssetMenu(menuName = "ScriptObj/MyScriptableObject")]
public class MyScriptableObj : ScriptableObject
{
    public string Str1;
    public int Int1;
    public MyField Field1;

    [SerializeField] private Color color1;


    [MenuItem("MyMenuItem/ScriptObj/MyScriptableObject", true)]
    public static bool CheckScriptableObj() {
        return Selection.activeObject != null 
            && Selection.activeObject.GetType() == typeof(MyScriptableObj);
    }
    [MenuItem("MyMenuItem/ScriptObj/MyScriptableObject")]
    public static void CreateScriptableObj() {
        var asset = ScriptableObject.CreateInstance<MyScriptableObj>();
    }

    [MenuItem("Assets/Create/ScriptObj/MyScriptableObject_Asset")]
    public static void CreateScriptableObj_Asset() {
        var asset = ScriptableObject.CreateInstance<MyScriptableObj>();
    }

    [ContextMenu("ResetInt1")]
    void ResetInt1() {
        Int1 = 0;
    }
}

[CustomEditor(typeof(MyScriptableObj))]
public class MyScriptableObjEditor : Editor
{
    MyScriptableObj mTarget;

    void OnEnable()
    {
        mTarget = (MyScriptableObj)target;
    }

    public override bool HasPreviewGUI(){
        return true;
    }
    public override void OnPreviewGUI(Rect r, GUIStyle background){
        GUI.Button(r, new GUIContent("Button"));
    }
    public override GUIContent GetPreviewTitle(){
        return new GUIContent("MyScriptableObj Preview");
    }
}

