' Add an Imports statement at the top of the class, structure, or
' module that uses the DllImport attribute.

Public Class Form1

    Public Declare Unicode Function Everything_SetSearchW Lib "d:\dev\everything\sdk\dll\Everything32.dll" (ByVal search As String) As UInt32
    Public Declare Unicode Function Everything_SetRequestFlags Lib "d:\dev\everything\sdk\dll\Everything32.dll" (ByVal dwRequestFlags As UInt32) As UInt32
    Public Declare Unicode Function Everything_QueryW Lib "d:\dev\everything\sdk\dll\Everything32.dll" (ByVal bWait As Integer) As Integer
    Public Declare Unicode Function Everything_GetNumResults Lib "d:\dev\everything\sdk\dll\Everything32.dll" () As UInt32
    Public Declare Unicode Function Everything_GetResultFileNameW Lib "d:\dev\everything\sdk\dll\Everything32.dll" (ByVal index As UInt32) As IntPtr
    Public Declare Unicode Function Everything_GetLastError Lib "d:\dev\everything\sdk\dll\Everything32.dll" () As UInt32
    Public Declare Unicode Function Everything_GetResultFullPathNameW Lib "d:\dev\everything\sdk\dll\Everything32.dll" (ByVal index As UInt32, ByVal buf As System.Text.StringBuilder, ByVal size As UInt32) As UInt32
    Public Declare Unicode Function Everything_GetResultSize Lib "d:\dev\everything\sdk\dll\Everything32.dll" (ByVal index As UInt32, ByRef size As UInt64) As Integer
    Public Declare Unicode Function Everything_GetResultDateModified Lib "d:\dev\everything\sdk\dll\Everything32.dll" (ByVal index As UInt32, ByRef ft As UInt64) As Integer

    Public Const EVERYTHING_REQUEST_FILE_NAME = &H1
    Public Const EVERYTHING_REQUEST_PATH                                 = &H00000002
    Public Const EVERYTHING_REQUEST_FULL_PATH_AND_FILE_NAME              = &H00000004
    Public Const EVERYTHING_REQUEST_EXTENSION                            = &H00000008
    Public Const EVERYTHING_REQUEST_SIZE                                 = &H00000010
    Public Const EVERYTHING_REQUEST_DATE_CREATED                         = &H00000020
    Public Const EVERYTHING_REQUEST_DATE_MODIFIED                        = &H00000040
    Public Const EVERYTHING_REQUEST_DATE_ACCESSED                        = &H00000080
    Public Const EVERYTHING_REQUEST_ATTRIBUTES                           = &H00000100
    Public Const EVERYTHING_REQUEST_FILE_LIST_FILE_NAME                  = &H00000200
    Public Const EVERYTHING_REQUEST_RUN_COUNT                            = &H00000400
    Public Const EVERYTHING_REQUEST_DATE_RUN                             = &H00000800
    Public Const EVERYTHING_REQUEST_DATE_RECENTLY_CHANGED                = &H00001000
    Public Const EVERYTHING_REQUEST_HIGHLIGHTED_FILE_NAME                = &H00002000
    Public Const EVERYTHING_REQUEST_HIGHLIGHTED_PATH                     = &H00004000
    Public Const EVERYTHING_REQUEST_HIGHLIGHTED_FULL_PATH_AND_FILE_NAME  = &H00008000

    Private Sub Button1_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button1.Click

        Everything_SetSearchW(TextBox1.Text)
        Everything_SetRequestFlags(EVERYTHING_REQUEST_FILE_NAME Or EVERYTHING_REQUEST_PATH Or EVERYTHING_REQUEST_SIZE Or EVERYTHING_REQUEST_DATE_MODIFIED)
        Everything_QueryW(1)

        Dim NumResults As UInt32
        Dim i As UInt32
        Dim filename As New System.Text.StringBuilder(260)
        Dim size As UInt64
        Dim ftdm As UInt64
        Dim DateModified As System.DateTime

        NumResults = Everything_GetNumResults()

        ListBox1.Items.Clear()

        If NumResults > 0 Then
            For i = 0 To NumResults - 1

                Everything_GetResultFullPathNameW(i, filename, filename.Capacity)
                Everything_GetResultSize(i, size)
                Everything_GetResultDateModified(i, ftdm)

                DateModified = System.DateTime.FromFileTime(ftdm)

                '                ListBox1.Items.Insert(i, filename.ToString() & " size:" & size & " date:" & DateModified.ToString())
                ListBox1.Items.Insert(i, System.Runtime.InteropServices.Marshal.PtrToStringUni(Everything_GetResultFileNameW(i)) & " Size:" & size & " Date Modified:" & DateModified.ToString())

            Next
        End If

    End Sub

End Class
