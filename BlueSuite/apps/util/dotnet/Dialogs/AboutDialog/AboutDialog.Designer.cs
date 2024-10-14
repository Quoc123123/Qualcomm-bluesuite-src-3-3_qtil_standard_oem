namespace QTIL.HostTools.Common.Dialogs
{
    partial class AboutDialog
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
            this.LabelInformationalVersion = new System.Windows.Forms.Label();
            this.ButtonOK = new System.Windows.Forms.Button();
            this.LabelCopyright = new System.Windows.Forms.Label();
            this.LabelVersionNumber = new System.Windows.Forms.Label();
            this.LabelAssemblyProduct = new System.Windows.Forms.Label();
            this.PictureBox1 = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.PictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // LabelInformationalVersion
            // 
            this.LabelInformationalVersion.Location = new System.Drawing.Point(64, 59);
            this.LabelInformationalVersion.Name = "LabelInformationalVersion";
            this.LabelInformationalVersion.Size = new System.Drawing.Size(371, 45);
            this.LabelInformationalVersion.TabIndex = 11;
            this.LabelInformationalVersion.Text = "Informational Version";
            // 
            // ButtonOK
            // 
            this.ButtonOK.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.ButtonOK.Location = new System.Drawing.Point(360, 9);
            this.ButtonOK.Name = "ButtonOK";
            this.ButtonOK.Size = new System.Drawing.Size(75, 23);
            this.ButtonOK.TabIndex = 10;
            this.ButtonOK.Text = "OK";
            // 
            // LabelCopyright
            // 
            this.LabelCopyright.Location = new System.Drawing.Point(64, 104);
            this.LabelCopyright.Name = "LabelCopyright";
            this.LabelCopyright.Size = new System.Drawing.Size(371, 41);
            this.LabelCopyright.TabIndex = 9;
            this.LabelCopyright.Text = "Copyright";
            // 
            // LabelVersionNumber
            // 
            this.LabelVersionNumber.Location = new System.Drawing.Point(64, 37);
            this.LabelVersionNumber.Name = "LabelVersionNumber";
            this.LabelVersionNumber.Size = new System.Drawing.Size(290, 20);
            this.LabelVersionNumber.TabIndex = 8;
            this.LabelVersionNumber.Text = "Version";
            // 
            // LabelAssemblyProduct
            // 
            this.LabelAssemblyProduct.Location = new System.Drawing.Point(64, 13);
            this.LabelAssemblyProduct.Name = "LabelAssemblyProduct";
            this.LabelAssemblyProduct.Size = new System.Drawing.Size(290, 16);
            this.LabelAssemblyProduct.TabIndex = 7;
            this.LabelAssemblyProduct.Text = "Assembly Product";
            // 
            // PictureBox1
            // 
            this.PictureBox1.Image = global::QTIL.HostTools.Common.Dialogs.Properties.Resources.Company;
            this.PictureBox1.Location = new System.Drawing.Point(10, 9);
            this.PictureBox1.Name = "PictureBox1";
            this.PictureBox1.Size = new System.Drawing.Size(48, 48);
            this.PictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.PictureBox1.TabIndex = 6;
            this.PictureBox1.TabStop = false;
            // 
            // AboutDialog
            // 
            this.AcceptButton = this.ButtonOK;
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Inherit;
            this.CancelButton = this.ButtonOK;
            this.ClientSize = new System.Drawing.Size(447, 154);
            this.Controls.Add(this.LabelInformationalVersion);
            this.Controls.Add(this.ButtonOK);
            this.Controls.Add(this.LabelCopyright);
            this.Controls.Add(this.LabelVersionNumber);
            this.Controls.Add(this.LabelAssemblyProduct);
            this.Controls.Add(this.PictureBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AboutDialog";
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "About temp";
            this.TopMost = true;
            ((System.ComponentModel.ISupportInitialize)(this.PictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        internal System.Windows.Forms.Label LabelInformationalVersion;
        internal System.Windows.Forms.Button ButtonOK;
        internal System.Windows.Forms.Label LabelCopyright;
        internal System.Windows.Forms.Label LabelVersionNumber;
        internal System.Windows.Forms.Label LabelAssemblyProduct;
        internal System.Windows.Forms.PictureBox PictureBox1;
    }
}
