using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace WindowsApplication1
{
	public partial class Form1 : Form
	{
		const int EVERYTHING_OK	= 0;
		const int EVERYTHING_ERROR_MEMORY = 1;
		const int EVERYTHING_ERROR_IPC = 2;
		const int EVERYTHING_ERROR_REGISTERCLASSEX = 3;
		const int EVERYTHING_ERROR_CREATEWINDOW = 4;
		const int EVERYTHING_ERROR_CREATETHREAD = 5;
		const int EVERYTHING_ERROR_INVALIDINDEX = 6;
		const int EVERYTHING_ERROR_INVALIDCALL = 7;

		const int EVERYTHING_REQUEST_FILE_NAME = 0x00000001;
		const int EVERYTHING_REQUEST_PATH = 0x00000002;
		const int EVERYTHING_REQUEST_FULL_PATH_AND_FILE_NAME = 0x00000004;
		const int EVERYTHING_REQUEST_EXTENSION = 0x00000008;
		const int EVERYTHING_REQUEST_SIZE = 0x00000010;
		const int EVERYTHING_REQUEST_DATE_CREATED = 0x00000020;
		const int EVERYTHING_REQUEST_DATE_MODIFIED = 0x00000040;
		const int EVERYTHING_REQUEST_DATE_ACCESSED = 0x00000080;
		const int EVERYTHING_REQUEST_ATTRIBUTES = 0x00000100;
		const int EVERYTHING_REQUEST_FILE_LIST_FILE_NAME = 0x00000200;
		const int EVERYTHING_REQUEST_RUN_COUNT = 0x00000400;
		const int EVERYTHING_REQUEST_DATE_RUN = 0x00000800;
		const int EVERYTHING_REQUEST_DATE_RECENTLY_CHANGED = 0x00001000;
		const int EVERYTHING_REQUEST_HIGHLIGHTED_FILE_NAME = 0x00002000;
		const int EVERYTHING_REQUEST_HIGHLIGHTED_PATH = 0x00004000;
		const int EVERYTHING_REQUEST_HIGHLIGHTED_FULL_PATH_AND_FILE_NAME = 0x00008000;

		const int EVERYTHING_SORT_NAME_ASCENDING = 1;
		const int EVERYTHING_SORT_NAME_DESCENDING = 2;
		const int EVERYTHING_SORT_PATH_ASCENDING = 3;
		const int EVERYTHING_SORT_PATH_DESCENDING = 4;
		const int EVERYTHING_SORT_SIZE_ASCENDING = 5;
		const int EVERYTHING_SORT_SIZE_DESCENDING = 6;
		const int EVERYTHING_SORT_EXTENSION_ASCENDING = 7;
		const int EVERYTHING_SORT_EXTENSION_DESCENDING = 8;
		const int EVERYTHING_SORT_TYPE_NAME_ASCENDING = 9;
		const int EVERYTHING_SORT_TYPE_NAME_DESCENDING = 10;
		const int EVERYTHING_SORT_DATE_CREATED_ASCENDING = 11;
		const int EVERYTHING_SORT_DATE_CREATED_DESCENDING = 12;
		const int EVERYTHING_SORT_DATE_MODIFIED_ASCENDING = 13;
		const int EVERYTHING_SORT_DATE_MODIFIED_DESCENDING = 14;
		const int EVERYTHING_SORT_ATTRIBUTES_ASCENDING = 15;
		const int EVERYTHING_SORT_ATTRIBUTES_DESCENDING = 16;
		const int EVERYTHING_SORT_FILE_LIST_FILENAME_ASCENDING = 17;
		const int EVERYTHING_SORT_FILE_LIST_FILENAME_DESCENDING = 18;
		const int EVERYTHING_SORT_RUN_COUNT_ASCENDING = 19;
		const int EVERYTHING_SORT_RUN_COUNT_DESCENDING = 20;
		const int EVERYTHING_SORT_DATE_RECENTLY_CHANGED_ASCENDING = 21;
		const int EVERYTHING_SORT_DATE_RECENTLY_CHANGED_DESCENDING = 22;
		const int EVERYTHING_SORT_DATE_ACCESSED_ASCENDING = 23;
		const int EVERYTHING_SORT_DATE_ACCESSED_DESCENDING = 24;
		const int EVERYTHING_SORT_DATE_RUN_ASCENDING = 25;
		const int EVERYTHING_SORT_DATE_RUN_DESCENDING = 26;

		const int EVERYTHING_TARGET_MACHINE_X86 = 1;
		const int EVERYTHING_TARGET_MACHINE_X64 = 2;
		const int EVERYTHING_TARGET_MACHINE_ARM = 3;

        [DllImport("Everything64.dll", CharSet = CharSet.Unicode)]
		public static extern UInt32 Everything_SetSearchW(string lpSearchString);
		[DllImport("Everything64.dll")]
		public static extern void Everything_SetMatchPath(bool bEnable);
		[DllImport("Everything64.dll")]
		public static extern void Everything_SetMatchCase(bool bEnable);
		[DllImport("Everything64.dll")]
		public static extern void Everything_SetMatchWholeWord(bool bEnable);
		[DllImport("Everything64.dll")]
		public static extern void Everything_SetRegex(bool bEnable);
		[DllImport("Everything64.dll")]
		public static extern void Everything_SetMax(UInt32 dwMax);
		[DllImport("Everything64.dll")]
		public static extern void Everything_SetOffset(UInt32 dwOffset);

		[DllImport("Everything64.dll")]
		public static extern bool Everything_GetMatchPath();
		[DllImport("Everything64.dll")]
		public static extern bool Everything_GetMatchCase();
		[DllImport("Everything64.dll")]
		public static extern bool Everything_GetMatchWholeWord();
		[DllImport("Everything64.dll")]
		public static extern bool Everything_GetRegex();
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetMax();
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetOffset();
		[DllImport("Everything64.dll")]
		public static extern IntPtr Everything_GetSearchW();
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetLastError();

		[DllImport("Everything64.dll")]
		public static extern bool Everything_QueryW(bool bWait);

		[DllImport("Everything64.dll")]
		public static extern void Everything_SortResultsByPath();

		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetNumFileResults();
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetNumFolderResults();
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetNumResults();
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetTotFileResults();
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetTotFolderResults();
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetTotResults();
		[DllImport("Everything64.dll")]
		public static extern bool Everything_IsVolumeResult(UInt32 nIndex);
		[DllImport("Everything64.dll")]
		public static extern bool Everything_IsFolderResult(UInt32 nIndex);
		[DllImport("Everything64.dll")]
		public static extern bool Everything_IsFileResult(UInt32 nIndex);
        [DllImport("Everything64.dll", CharSet = CharSet.Unicode)]
		public static extern void Everything_GetResultFullPathName(UInt32 nIndex, StringBuilder lpString, UInt32 nMaxCount);
		[DllImport("Everything64.dll")]
		public static extern void Everything_Reset();

        [DllImport("Everything64.dll", CharSet = CharSet.Unicode)]
		public static extern IntPtr Everything_GetResultFileName(UInt32 nIndex);

		// Everything 1.4
		[DllImport("Everything64.dll")]
		public static extern void Everything_SetSort(UInt32 dwSortType);
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetSort();
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetResultListSort();
		[DllImport("Everything64.dll")]
		public static extern void Everything_SetRequestFlags(UInt32 dwRequestFlags);
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetRequestFlags();
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetResultListRequestFlags();
		[DllImport("Everything64.dll", CharSet = CharSet.Unicode)]
		public static extern IntPtr Everything_GetResultExtension(UInt32 nIndex);
		[DllImport("Everything64.dll")]
		public static extern bool Everything_GetResultSize(UInt32 nIndex, out long lpFileSize);
		[DllImport("Everything64.dll")]
		public static extern bool Everything_GetResultDateCreated(UInt32 nIndex, out long lpFileTime);
		[DllImport("Everything64.dll")]
		public static extern bool Everything_GetResultDateModified(UInt32 nIndex, out long lpFileTime);
		[DllImport("Everything64.dll")]
		public static extern bool Everything_GetResultDateAccessed(UInt32 nIndex, out long lpFileTime);
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetResultAttributes(UInt32 nIndex);
		[DllImport("Everything64.dll", CharSet = CharSet.Unicode)]
		public static extern IntPtr Everything_GetResultFileListFileName(UInt32 nIndex);
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetResultRunCount(UInt32 nIndex);
		[DllImport("Everything64.dll")]
		public static extern bool Everything_GetResultDateRun(UInt32 nIndex, out long lpFileTime);
		[DllImport("Everything64.dll")]
		public static extern bool Everything_GetResultDateRecentlyChanged(UInt32 nIndex, out long lpFileTime);
		[DllImport("Everything64.dll", CharSet = CharSet.Unicode)]
		public static extern IntPtr Everything_GetResultHighlightedFileName(UInt32 nIndex);
		[DllImport("Everything64.dll", CharSet = CharSet.Unicode)]
		public static extern IntPtr Everything_GetResultHighlightedPath(UInt32 nIndex);
		[DllImport("Everything64.dll", CharSet = CharSet.Unicode)]
		public static extern IntPtr Everything_GetResultHighlightedFullPathAndFileName(UInt32 nIndex);
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_GetRunCountFromFileName(string lpFileName);
		[DllImport("Everything64.dll")]
		public static extern bool Everything_SetRunCountFromFileName(string lpFileName, UInt32 dwRunCount);
		[DllImport("Everything64.dll")]
		public static extern UInt32 Everything_IncRunCountFromFileName(string lpFileName);

        public Form1()
		{
			InitializeComponent();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			UInt32 i;

			// set the search
			Everything_SetSearchW(textBox1.Text);

			// use our own custom scrollbar... 			
			// Everything_SetMax(listBox1.ClientRectangle.Height / listBox1.ItemHeight);
			// Everything_SetOffset(VerticalScrollBarPosition...);

			// request name and size
			Everything_SetRequestFlags(EVERYTHING_REQUEST_FILE_NAME | EVERYTHING_REQUEST_PATH | EVERYTHING_REQUEST_DATE_MODIFIED | EVERYTHING_REQUEST_SIZE);

			Everything_SetSort(13);

			// execute the query
			Everything_QueryW(true);

			// sort by path
			// Everything_SortResultsByPath();

			// clear the old list of results			
			listBox1.Items.Clear();

			// set the window title
			Text = textBox1.Text + " - " + Everything_GetNumResults() + " Results";
			
			// loop through the results, adding each result to the listbox.
			for (i = 0; i < Everything_GetNumResults(); i++)
			{
				long date_modified;
				long size;

				Everything_GetResultDateModified(i, out date_modified);
				Everything_GetResultSize(i, out size);

				// add it to the list box				
				listBox1.Items.Insert((int)i, "Size: " + size.ToString() + " Date Modified: " + DateTime.FromFileTime(date_modified).Year + "/" + DateTime.FromFileTime(date_modified).Month + "/" + DateTime.FromFileTime(date_modified).Day + " " + DateTime.FromFileTime(date_modified).Hour + ":" + DateTime.FromFileTime(date_modified).Minute.ToString("D2") + " Filename: " + Marshal.PtrToStringUni(Everything_GetResultFileName(i)));
			}
		}
	}
}