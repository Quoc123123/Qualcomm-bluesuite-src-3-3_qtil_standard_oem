namespace QTIL.HostTools.Common.Transport
{
    partial class TransportDialog
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.ComboBoxOption = new System.Windows.Forms.ComboBox();
            this.LabelOption = new System.Windows.Forms.Label();
            this.LabelPort = new System.Windows.Forms.Label();
            this.LabelTransport = new System.Windows.Forms.Label();
            this.MainMenu = new System.Windows.Forms.MenuStrip();
            this.HistoryMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ButtonOK = new System.Windows.Forms.Button();
            this.ButtonCancel = new System.Windows.Forms.Button();
            this.LabelOnlyTransport = new System.Windows.Forms.Label();
            this.ComboBoxTransports = new System.Windows.Forms.ComboBox();
            this.LabelOnlyDevice = new System.Windows.Forms.Label();
            this.ComboBoxPorts = new System.Windows.Forms.ComboBox();
            this.MainMenu.SuspendLayout();
            this.SuspendLayout();
            // 
            // ComboBoxOption
            // 
            this.ComboBoxOption.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.ComboBoxOption.Location = new System.Drawing.Point(89, 94);
            this.ComboBoxOption.Name = "ComboBoxOption";
            this.ComboBoxOption.Size = new System.Drawing.Size(157, 21);
            this.ComboBoxOption.TabIndex = 29;
            this.ComboBoxOption.Visible = false;
            this.ComboBoxOption.SelectedIndexChanged += new System.EventHandler(this.ComboBoxOption_SelectedIndexChanged);
            // 
            // LabelOption
            // 
            this.LabelOption.Location = new System.Drawing.Point(9, 92);
            this.LabelOption.Name = "LabelOption";
            this.LabelOption.Size = new System.Drawing.Size(75, 23);
            this.LabelOption.TabIndex = 26;
            this.LabelOption.Text = "Options";
            this.LabelOption.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // LabelPort
            // 
            this.LabelPort.Location = new System.Drawing.Point(9, 65);
            this.LabelPort.Name = "LabelPort";
            this.LabelPort.Size = new System.Drawing.Size(75, 23);
            this.LabelPort.TabIndex = 25;
            this.LabelPort.Text = "Device";
            this.LabelPort.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // LabelTransport
            // 
            this.LabelTransport.Location = new System.Drawing.Point(9, 38);
            this.LabelTransport.Name = "LabelTransport";
            this.LabelTransport.Size = new System.Drawing.Size(75, 23);
            this.LabelTransport.TabIndex = 24;
            this.LabelTransport.Text = "Transport";
            this.LabelTransport.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // MainMenu
            // 
            this.MainMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.HistoryMenuItem});
            this.MainMenu.Location = new System.Drawing.Point(0, 0);
            this.MainMenu.Name = "MainMenu";
            this.MainMenu.Size = new System.Drawing.Size(343, 24);
            this.MainMenu.TabIndex = 23;
            this.MainMenu.Text = "MainMenu";
            // 
            // HistoryMenuItem
            // 
            this.HistoryMenuItem.Enabled = false;
            this.HistoryMenuItem.Name = "HistoryMenuItem";
            this.HistoryMenuItem.Size = new System.Drawing.Size(53, 20);
            this.HistoryMenuItem.Text = "&History";
            this.HistoryMenuItem.DropDownItemClicked += new System.Windows.Forms.ToolStripItemClickedEventHandler(this.HistoryMenuItem_DropDownItemClicked);
            this.HistoryMenuItem.DropDownOpening += new System.EventHandler(this.HistoryMenuItem_DropDownOpening);
            // 
            // ButtonOK
            // 
            this.ButtonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.ButtonOK.Enabled = false;
            this.ButtonOK.Location = new System.Drawing.Point(269, 38);
            this.ButtonOK.Name = "ButtonOK";
            this.ButtonOK.Size = new System.Drawing.Size(67, 23);
            this.ButtonOK.TabIndex = 22;
            this.ButtonOK.Text = "OK";
            this.ButtonOK.Click += new System.EventHandler(this.ButtonOK_Click);
            // 
            // ButtonCancel
            // 
            this.ButtonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.ButtonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.ButtonCancel.Location = new System.Drawing.Point(269, 65);
            this.ButtonCancel.Name = "ButtonCancel";
            this.ButtonCancel.Size = new System.Drawing.Size(67, 23);
            this.ButtonCancel.TabIndex = 21;
            this.ButtonCancel.Text = "Cancel";
            this.ButtonCancel.Click += new System.EventHandler(this.ButtonCancel_Click);
            // 
            // LabelOnlyTransport
            // 
            this.LabelOnlyTransport.BackColor = System.Drawing.SystemColors.Window;
            this.LabelOnlyTransport.Location = new System.Drawing.Point(91, 42);
            this.LabelOnlyTransport.Name = "LabelOnlyTransport";
            this.LabelOnlyTransport.Size = new System.Drawing.Size(136, 17);
            this.LabelOnlyTransport.TabIndex = 30;
            this.LabelOnlyTransport.Text = "Only Transport";
            this.LabelOnlyTransport.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.LabelOnlyTransport.Visible = false;
            // 
            // ComboBoxTransports
            // 
            this.ComboBoxTransports.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.ComboBoxTransports.Location = new System.Drawing.Point(89, 40);
            this.ComboBoxTransports.Name = "ComboBoxTransports";
            this.ComboBoxTransports.Size = new System.Drawing.Size(157, 21);
            this.ComboBoxTransports.TabIndex = 27;
            this.ComboBoxTransports.SelectedIndexChanged += new System.EventHandler(this.ComboBoxTransports_SelectedIndexChanged);
            // 
            // LabelOnlyDevice
            // 
            this.LabelOnlyDevice.BackColor = System.Drawing.SystemColors.Window;
            this.LabelOnlyDevice.Location = new System.Drawing.Point(91, 69);
            this.LabelOnlyDevice.Name = "LabelOnlyDevice";
            this.LabelOnlyDevice.Size = new System.Drawing.Size(136, 17);
            this.LabelOnlyDevice.TabIndex = 31;
            this.LabelOnlyDevice.Text = "Only Device";
            this.LabelOnlyDevice.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.LabelOnlyDevice.Visible = false;
            // 
            // ComboBoxPorts
            // 
            this.ComboBoxPorts.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.ComboBoxPorts.Location = new System.Drawing.Point(89, 67);
            this.ComboBoxPorts.Name = "ComboBoxPorts";
            this.ComboBoxPorts.Size = new System.Drawing.Size(157, 21);
            this.ComboBoxPorts.TabIndex = 28;
            this.ComboBoxPorts.Visible = false;
            this.ComboBoxPorts.SelectedIndexChanged += new System.EventHandler(this.ComboBoxPorts_SelectedIndexChanged);
            this.ComboBoxPorts.DropDown += new System.EventHandler(this.ComboBoxPorts_DropDown);
            // 
            // TransportDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(343, 119);
            this.Controls.Add(this.ComboBoxOption);
            this.Controls.Add(this.LabelOption);
            this.Controls.Add(this.LabelPort);
            this.Controls.Add(this.LabelTransport);
            this.Controls.Add(this.MainMenu);
            this.Controls.Add(this.ButtonOK);
            this.Controls.Add(this.ButtonCancel);
            this.Controls.Add(this.LabelOnlyTransport);
            this.Controls.Add(this.ComboBoxTransports);
            this.Controls.Add(this.LabelOnlyDevice);
            this.Controls.Add(this.ComboBoxPorts);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "TransportDialog";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Transport Dialog";
            this.MainMenu.ResumeLayout(false);
            this.MainMenu.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        internal System.Windows.Forms.ComboBox ComboBoxOption;
        internal System.Windows.Forms.Label LabelOption;
        internal System.Windows.Forms.Label LabelPort;
        internal System.Windows.Forms.Label LabelTransport;
        internal System.Windows.Forms.MenuStrip MainMenu;
        internal System.Windows.Forms.ToolStripMenuItem HistoryMenuItem;
        internal System.Windows.Forms.Button ButtonOK;
        internal System.Windows.Forms.Button ButtonCancel;
        internal System.Windows.Forms.Label LabelOnlyTransport;
        internal System.Windows.Forms.ComboBox ComboBoxTransports;
        internal System.Windows.Forms.Label LabelOnlyDevice;
        internal System.Windows.Forms.ComboBox ComboBoxPorts;
    }
}
