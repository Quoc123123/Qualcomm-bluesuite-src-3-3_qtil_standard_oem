namespace QTIL.HostTools.HidDfuApp
{
    partial class MainForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.prgBar = new System.Windows.Forms.ProgressBar();
            this.cmbBoxCommand = new System.Windows.Forms.ComboBox();
            this.lblFilename = new System.Windows.Forms.Label();
            this.btnFileName = new System.Windows.Forms.Button();
            this.lblProgress = new System.Windows.Forms.Label();
            this.grpBox = new System.Windows.Forms.GroupBox();
            this.lblHexUsagePage = new System.Windows.Forms.Label();
            this.lblHexUsage = new System.Windows.Forms.Label();
            this.lblHexPid = new System.Windows.Forms.Label();
            this.lblHexVid = new System.Windows.Forms.Label();
            this.txtBoxFileName = new System.Windows.Forms.TextBox();
            this.lblUsagePage = new System.Windows.Forms.Label();
            this.lblUsage = new System.Windows.Forms.Label();
            this.lblPid = new System.Windows.Forms.Label();
            this.lblVid = new System.Windows.Forms.Label();
            this.numUpDnUsagePage = new System.Windows.Forms.NumericUpDown();
            this.numUpDnUsage = new System.Windows.Forms.NumericUpDown();
            this.numUpDnPid = new System.Windows.Forms.NumericUpDown();
            this.numUpDnVid = new System.Windows.Forms.NumericUpDown();
            this.chkBoxReset = new System.Windows.Forms.CheckBox();
            this.lblCommand = new System.Windows.Forms.Label();
            this.btnHelp = new System.Windows.Forms.Button();
            this.btnRun = new System.Windows.Forms.Button();
            this.grpBoxProgress = new System.Windows.Forms.GroupBox();
            this.btnStop = new System.Windows.Forms.Button();
            this.txtBoxLog = new System.Windows.Forms.TextBox();
            this.backgrndWrkr = new System.ComponentModel.BackgroundWorker();
            this.grpBox.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numUpDnUsagePage)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numUpDnUsage)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numUpDnPid)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numUpDnVid)).BeginInit();
            this.grpBoxProgress.SuspendLayout();
            this.SuspendLayout();
            // 
            // prgBar
            // 
            this.prgBar.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.prgBar.Location = new System.Drawing.Point(96, 23);
            this.prgBar.Name = "prgBar";
            this.prgBar.Size = new System.Drawing.Size(369, 25);
            this.prgBar.TabIndex = 10;
            // 
            // cmbBoxCommand
            // 
            this.cmbBoxCommand.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbBoxCommand.Location = new System.Drawing.Point(106, 35);
            this.cmbBoxCommand.Name = "cmbBoxCommand";
            this.cmbBoxCommand.Size = new System.Drawing.Size(106, 21);
            this.cmbBoxCommand.TabIndex = 25;
            this.cmbBoxCommand.SelectedIndexChanged += new System.EventHandler(this.cmbBoxCommand_SelectedIndexChanged);
            // 
            // lblFilename
            // 
            this.lblFilename.AutoSize = true;
            this.lblFilename.Location = new System.Drawing.Point(18, 85);
            this.lblFilename.Name = "lblFilename";
            this.lblFilename.Size = new System.Drawing.Size(23, 13);
            this.lblFilename.TabIndex = 5;
            this.lblFilename.Text = "File";
            // 
            // btnFileName
            // 
            this.btnFileName.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnFileName.Location = new System.Drawing.Point(383, 80);
            this.btnFileName.Name = "btnFileName";
            this.btnFileName.Size = new System.Drawing.Size(80, 23);
            this.btnFileName.TabIndex = 4;
            this.btnFileName.Text = "Choose File...";
            this.btnFileName.UseVisualStyleBackColor = true;
            this.btnFileName.Click += new System.EventHandler(this.btnFileName_Click);
            // 
            // lblProgress
            // 
            this.lblProgress.AutoSize = true;
            this.lblProgress.Location = new System.Drawing.Point(14, 29);
            this.lblProgress.Name = "lblProgress";
            this.lblProgress.Size = new System.Drawing.Size(48, 13);
            this.lblProgress.TabIndex = 13;
            this.lblProgress.Text = "Progress";
            // 
            // grpBox
            // 
            this.grpBox.Controls.Add(this.lblHexUsagePage);
            this.grpBox.Controls.Add(this.lblHexUsage);
            this.grpBox.Controls.Add(this.lblHexPid);
            this.grpBox.Controls.Add(this.lblHexVid);
            this.grpBox.Controls.Add(this.txtBoxFileName);
            this.grpBox.Controls.Add(this.lblUsagePage);
            this.grpBox.Controls.Add(this.lblUsage);
            this.grpBox.Controls.Add(this.lblPid);
            this.grpBox.Controls.Add(this.lblVid);
            this.grpBox.Controls.Add(this.numUpDnUsagePage);
            this.grpBox.Controls.Add(this.numUpDnUsage);
            this.grpBox.Controls.Add(this.numUpDnPid);
            this.grpBox.Controls.Add(this.numUpDnVid);
            this.grpBox.Controls.Add(this.chkBoxReset);
            this.grpBox.Controls.Add(this.btnFileName);
            this.grpBox.Controls.Add(this.lblCommand);
            this.grpBox.Controls.Add(this.cmbBoxCommand);
            this.grpBox.Controls.Add(this.lblFilename);
            this.grpBox.Location = new System.Drawing.Point(12, 14);
            this.grpBox.Name = "grpBox";
            this.grpBox.Size = new System.Drawing.Size(483, 221);
            this.grpBox.TabIndex = 14;
            this.grpBox.TabStop = false;
            this.grpBox.Text = "Operation";
            // 
            // lblHexUsagePage
            // 
            this.lblHexUsagePage.AutoSize = true;
            this.lblHexUsagePage.Location = new System.Drawing.Point(338, 176);
            this.lblHexUsagePage.Name = "lblHexUsagePage";
            this.lblHexUsagePage.Size = new System.Drawing.Size(18, 13);
            this.lblHexUsagePage.TabIndex = 30;
            this.lblHexUsagePage.Text = "0x";
            // 
            // lblHexUsage
            // 
            this.lblHexUsage.AutoSize = true;
            this.lblHexUsage.Location = new System.Drawing.Point(86, 176);
            this.lblHexUsage.Name = "lblHexUsage";
            this.lblHexUsage.Size = new System.Drawing.Size(18, 13);
            this.lblHexUsage.TabIndex = 29;
            this.lblHexUsage.Text = "0x";
            // 
            // lblHexPid
            // 
            this.lblHexPid.AutoSize = true;
            this.lblHexPid.Location = new System.Drawing.Point(338, 131);
            this.lblHexPid.Name = "lblHexPid";
            this.lblHexPid.Size = new System.Drawing.Size(18, 13);
            this.lblHexPid.TabIndex = 28;
            this.lblHexPid.Text = "0x";
            // 
            // lblHexVid
            // 
            this.lblHexVid.AutoSize = true;
            this.lblHexVid.Location = new System.Drawing.Point(86, 130);
            this.lblHexVid.Name = "lblHexVid";
            this.lblHexVid.Size = new System.Drawing.Size(18, 13);
            this.lblHexVid.TabIndex = 27;
            this.lblHexVid.Text = "0x";
            // 
            // txtBoxFileName
            // 
            this.txtBoxFileName.Location = new System.Drawing.Point(106, 81);
            this.txtBoxFileName.Name = "txtBoxFileName";
            this.txtBoxFileName.Size = new System.Drawing.Size(278, 20);
            this.txtBoxFileName.TabIndex = 26;
            this.txtBoxFileName.TextChanged += new System.EventHandler(this.txtBoxFileName_TextChanged);
            // 
            // lblUsagePage
            // 
            this.lblUsagePage.AutoSize = true;
            this.lblUsagePage.Location = new System.Drawing.Point(250, 177);
            this.lblUsagePage.Name = "lblUsagePage";
            this.lblUsagePage.Size = new System.Drawing.Size(66, 13);
            this.lblUsagePage.TabIndex = 23;
            this.lblUsagePage.Text = "Usage Page";
            // 
            // lblUsage
            // 
            this.lblUsage.AutoSize = true;
            this.lblUsage.Location = new System.Drawing.Point(18, 177);
            this.lblUsage.Name = "lblUsage";
            this.lblUsage.Size = new System.Drawing.Size(38, 13);
            this.lblUsage.TabIndex = 22;
            this.lblUsage.Text = "Usage";
            // 
            // lblPid
            // 
            this.lblPid.AutoSize = true;
            this.lblPid.Location = new System.Drawing.Point(250, 131);
            this.lblPid.Name = "lblPid";
            this.lblPid.Size = new System.Drawing.Size(25, 13);
            this.lblPid.TabIndex = 21;
            this.lblPid.Text = "PID";
            // 
            // lblVid
            // 
            this.lblVid.AutoSize = true;
            this.lblVid.Location = new System.Drawing.Point(18, 131);
            this.lblVid.Name = "lblVid";
            this.lblVid.Size = new System.Drawing.Size(25, 13);
            this.lblVid.TabIndex = 20;
            this.lblVid.Text = "VID";
            // 
            // numUpDnUsagePage
            // 
            this.numUpDnUsagePage.Hexadecimal = true;
            this.numUpDnUsagePage.Location = new System.Drawing.Point(357, 173);
            this.numUpDnUsagePage.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.numUpDnUsagePage.Name = "numUpDnUsagePage";
            this.numUpDnUsagePage.Size = new System.Drawing.Size(106, 20);
            this.numUpDnUsagePage.TabIndex = 18;
            this.numUpDnUsagePage.ValueChanged += new System.EventHandler(this.numUpDnUsagePage_ValueChanged);
            // 
            // numUpDnUsage
            // 
            this.numUpDnUsage.Hexadecimal = true;
            this.numUpDnUsage.Location = new System.Drawing.Point(106, 173);
            this.numUpDnUsage.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.numUpDnUsage.Name = "numUpDnUsage";
            this.numUpDnUsage.Size = new System.Drawing.Size(106, 20);
            this.numUpDnUsage.TabIndex = 17;
            this.numUpDnUsage.ValueChanged += new System.EventHandler(this.numUpDnUsage_ValueChanged);
            // 
            // numUpDnPid
            // 
            this.numUpDnPid.Hexadecimal = true;
            this.numUpDnPid.Location = new System.Drawing.Point(357, 127);
            this.numUpDnPid.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.numUpDnPid.Name = "numUpDnPid";
            this.numUpDnPid.Size = new System.Drawing.Size(106, 20);
            this.numUpDnPid.TabIndex = 16;
            this.numUpDnPid.ValueChanged += new System.EventHandler(this.numUpDnPid_ValueChanged);
            // 
            // numUpDnVid
            // 
            this.numUpDnVid.Hexadecimal = true;
            this.numUpDnVid.Location = new System.Drawing.Point(106, 127);
            this.numUpDnVid.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.numUpDnVid.Name = "numUpDnVid";
            this.numUpDnVid.Size = new System.Drawing.Size(106, 20);
            this.numUpDnVid.TabIndex = 15;
            this.numUpDnVid.ValueChanged += new System.EventHandler(this.numUpDnVid_ValueChanged);
            // 
            // chkBoxReset
            // 
            this.chkBoxReset.AutoSize = true;
            this.chkBoxReset.Location = new System.Drawing.Point(358, 37);
            this.chkBoxReset.Name = "chkBoxReset";
            this.chkBoxReset.Size = new System.Drawing.Size(79, 17);
            this.chkBoxReset.TabIndex = 14;
            this.chkBoxReset.Text = "Reset After";
            this.chkBoxReset.TextAlign = System.Drawing.ContentAlignment.TopLeft;
            this.chkBoxReset.UseVisualStyleBackColor = true;
            this.chkBoxReset.CheckedChanged += new System.EventHandler(this.chkBoxReset_CheckedChanged);
            // 
            // lblCommand
            // 
            this.lblCommand.AutoSize = true;
            this.lblCommand.Location = new System.Drawing.Point(18, 39);
            this.lblCommand.Name = "lblCommand";
            this.lblCommand.Size = new System.Drawing.Size(54, 13);
            this.lblCommand.TabIndex = 10;
            this.lblCommand.Text = "Command";
            // 
            // btnHelp
            // 
            this.btnHelp.Location = new System.Drawing.Point(398, 59);
            this.btnHelp.Name = "btnHelp";
            this.btnHelp.Size = new System.Drawing.Size(66, 25);
            this.btnHelp.TabIndex = 27;
            this.btnHelp.Text = "Help";
            this.btnHelp.UseVisualStyleBackColor = true;
            this.btnHelp.Click += new System.EventHandler(this.btnHelp_Click);
            // 
            // btnRun
            // 
            this.btnRun.Location = new System.Drawing.Point(96, 59);
            this.btnRun.Name = "btnRun";
            this.btnRun.Size = new System.Drawing.Size(66, 25);
            this.btnRun.TabIndex = 9;
            this.btnRun.Text = "Run";
            this.btnRun.UseVisualStyleBackColor = true;
            this.btnRun.Click += new System.EventHandler(this.btnRun_Click);
            // 
            // grpBoxProgress
            // 
            this.grpBoxProgress.Controls.Add(this.btnStop);
            this.grpBoxProgress.Controls.Add(this.btnHelp);
            this.grpBoxProgress.Controls.Add(this.prgBar);
            this.grpBoxProgress.Controls.Add(this.lblProgress);
            this.grpBoxProgress.Controls.Add(this.btnRun);
            this.grpBoxProgress.Location = new System.Drawing.Point(12, 253);
            this.grpBoxProgress.Name = "grpBoxProgress";
            this.grpBoxProgress.Size = new System.Drawing.Size(483, 99);
            this.grpBoxProgress.TabIndex = 15;
            this.grpBoxProgress.TabStop = false;
            // 
            // btnStop
            // 
            this.btnStop.Location = new System.Drawing.Point(247, 59);
            this.btnStop.Name = "btnStop";
            this.btnStop.Size = new System.Drawing.Size(66, 25);
            this.btnStop.TabIndex = 28;
            this.btnStop.Text = "Stop";
            this.btnStop.UseVisualStyleBackColor = true;
            this.btnStop.Click += new System.EventHandler(this.btnStop_Click);
            // 
            // txtBoxLog
            // 
            this.txtBoxLog.Location = new System.Drawing.Point(12, 370);
            this.txtBoxLog.Multiline = true;
            this.txtBoxLog.Name = "txtBoxLog";
            this.txtBoxLog.ReadOnly = true;
            this.txtBoxLog.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.txtBoxLog.Size = new System.Drawing.Size(483, 262);
            this.txtBoxLog.TabIndex = 16;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(506, 642);
            this.Controls.Add(this.txtBoxLog);
            this.Controls.Add(this.grpBoxProgress);
            this.Controls.Add(this.grpBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimumSize = new System.Drawing.Size(500, 300);
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "HID DFU Application";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainPage_FormClosing);
            this.Shown += new System.EventHandler(this.MainPage_Shown);
            this.MouseClick += new System.Windows.Forms.MouseEventHandler(this.MainPage_MouseClick);
            this.grpBox.ResumeLayout(false);
            this.grpBox.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numUpDnUsagePage)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numUpDnUsage)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numUpDnPid)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numUpDnVid)).EndInit();
            this.grpBoxProgress.ResumeLayout(false);
            this.grpBoxProgress.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.ProgressBar prgBar;
        private System.Windows.Forms.ComboBox cmbBoxCommand;
        private System.Windows.Forms.Label lblFilename;
        private System.Windows.Forms.Button btnFileName;
        private System.Windows.Forms.Label lblProgress;
        private System.Windows.Forms.GroupBox grpBox;
        private System.Windows.Forms.Label lblUsagePage;
        private System.Windows.Forms.Label lblUsage;
        private System.Windows.Forms.Label lblPid;
        private System.Windows.Forms.Label lblVid;
        private System.Windows.Forms.NumericUpDown numUpDnUsagePage;
        private System.Windows.Forms.NumericUpDown numUpDnUsage;
        private System.Windows.Forms.NumericUpDown numUpDnPid;
        private System.Windows.Forms.NumericUpDown numUpDnVid;
        private System.Windows.Forms.CheckBox chkBoxReset;
        private System.Windows.Forms.Button btnRun;
        private System.Windows.Forms.Label lblCommand;
        private System.Windows.Forms.GroupBox grpBoxProgress;
        private System.Windows.Forms.TextBox txtBoxFileName;
        private System.Windows.Forms.Button btnHelp;
        private System.Windows.Forms.TextBox txtBoxLog;
        private System.Windows.Forms.Label lblHexUsagePage;
        private System.Windows.Forms.Label lblHexUsage;
        private System.Windows.Forms.Label lblHexPid;
        private System.Windows.Forms.Label lblHexVid;
        private System.Windows.Forms.Button btnStop;
        private System.ComponentModel.BackgroundWorker backgrndWrkr;
    }
}

