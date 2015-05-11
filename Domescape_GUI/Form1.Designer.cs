namespace Domescape
{
    partial class Form1
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
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.networkGroupBox = new System.Windows.Forms.GroupBox();
            this.connectButton = new System.Windows.Forms.Button();
			this.dateButton = new System.Windows.Forms.Button();
            this.ipTextBox = new System.Windows.Forms.TextBox();
			this.dateTextBox = new System.Windows.Forms.TextBox();
            this.adressLabel = new System.Windows.Forms.Label();
			this.dateLabel = new System.Windows.Forms.Label();
            this.ControlsGroupBox = new System.Windows.Forms.GroupBox();
			this.pauseButton = new System.Windows.Forms.Button();
			this.resetButton = new System.Windows.Forms.Button();
            this.SpeedTrackBar = new System.Windows.Forms.TrackBar();
            this.SpeedLabel = new System.Windows.Forms.Label();
            this.statusStrip1.SuspendLayout();
            this.networkGroupBox.SuspendLayout();
            this.ControlsGroupBox.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.SpeedTrackBar)).BeginInit();
            this.SuspendLayout();
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1});
            this.statusStrip1.Location = new System.Drawing.Point(0, 240);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(284, 22);
            this.statusStrip1.TabIndex = 0;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(118, 17);
            this.toolStripStatusLabel1.Text = "toolStripStatusLabel1";
            // 
            // networkGroupBox
            // 
            this.networkGroupBox.Controls.Add(this.connectButton);
            this.networkGroupBox.Controls.Add(this.ipTextBox);
            this.networkGroupBox.Controls.Add(this.adressLabel);
            this.networkGroupBox.Location = new System.Drawing.Point(200, 13);
			this.networkGroupBox.Name = "networkGroupBox";
            this.networkGroupBox.Size = new System.Drawing.Size(275, 48);
            this.networkGroupBox.TabIndex = 1;
            this.networkGroupBox.TabStop = false;
            this.networkGroupBox.Text = "Network";
            // 
            // connectButton
            // 
            this.connectButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.connectButton.Location = new System.Drawing.Point(151, 20);
            this.connectButton.Name = "connectButton";
            this.connectButton.Size = new System.Drawing.Size(102, 20);
            this.connectButton.TabIndex = 3;
            this.connectButton.Text = "Connect";
            this.connectButton.UseVisualStyleBackColor = true;
            this.connectButton.Click += new System.EventHandler(this.connectButton_Click);
            // 
            // ipTextBox
            // 
            this.ipTextBox.Location = new System.Drawing.Point(57, 17);
            this.ipTextBox.MaxLength = 16;
            this.ipTextBox.Name = "ipTextBox";
            this.ipTextBox.Size = new System.Drawing.Size(88, 20);
            this.ipTextBox.TabIndex = 2;
            this.ipTextBox.Text = "127.0.0.1";
            // 
            // adressLabel
            // 
            this.adressLabel.AutoSize = true;
            this.adressLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.adressLabel.Location = new System.Drawing.Point(8, 20);
            this.adressLabel.Name = "adressLabel";
            this.adressLabel.Size = new System.Drawing.Size(48, 13);
            this.adressLabel.TabIndex = 0;
			this.adressLabel.Text = "Address:";
            // 
            // ControlsGroupBox
            // 
			this.ControlsGroupBox.Controls.Add(this.dateLabel);
			this.ControlsGroupBox.Controls.Add(this.dateButton);
			this.ControlsGroupBox.Controls.Add(this.dateTextBox);
            this.ControlsGroupBox.Controls.Add(this.SpeedLabel);
            this.ControlsGroupBox.Controls.Add(this.SpeedTrackBar);
			this.ControlsGroupBox.Controls.Add(this.pauseButton);
			this.ControlsGroupBox.Controls.Add(this.resetButton);
            this.ControlsGroupBox.Location = new System.Drawing.Point(13, 68);
            this.ControlsGroupBox.Name = "ControlsGroupBox";
            this.ControlsGroupBox.Size = new System.Drawing.Size(622, 260);
            this.ControlsGroupBox.TabIndex = 4;
            this.ControlsGroupBox.TabStop = false;
            this.ControlsGroupBox.Text = "Controls";
			// 
			// datebutton
			// 
			this.dateButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.dateButton.Location = new System.Drawing.Point(350, 30);
			this.dateButton.Name = "dateButton";
			this.dateButton.Size = new System.Drawing.Size(130, 25);
			this.dateButton.TabIndex = 6;
			this.dateButton.Text = "Use date";
			this.dateButton.UseVisualStyleBackColor = true;
			this.dateButton.Click += new System.EventHandler(this.dateButton_Click);
			// 
			// dateTextBox
			// 
			this.dateTextBox.Location = new System.Drawing.Point(200, 30);
			this.dateTextBox.MaxLength = 20;
			this.dateTextBox.Name = "dateTextBox";
			this.dateTextBox.Size = new System.Drawing.Size(145, 25);
			this.dateTextBox.TabIndex = 5;
			this.dateTextBox.Text = "YYYY MMM DD HH:MM:SS";
			// 
			// dateLabel
			// 
			this.dateLabel.AutoSize = true;
			this.dateLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.dateLabel.Location = new System.Drawing.Point(162, 35);
			this.dateLabel.Name = "dateLabel";
			this.dateLabel.Size = new System.Drawing.Size(48, 13);
			this.dateLabel.TabIndex = 0;
			this.dateLabel.Text = "Date:";		
			// 
			// resetbutton
			// 
			this.resetButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.resetButton.Location = new System.Drawing.Point(50, 100);
			this.resetButton.Name = "resetButton";
			this.resetButton.Size = new System.Drawing.Size(130, 40);
			this.resetButton.TabIndex = 7;
			this.resetButton.Text = "Reset to current time";
			this.resetButton.UseVisualStyleBackColor = true;
			this.resetButton.Click += new System.EventHandler(this.resetButton_Click);
			// 
			// pausebutton
			// 
			this.pauseButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.pauseButton.Location = new System.Drawing.Point(450, 100);
			this.pauseButton.Name = "dateButton";
			this.pauseButton.Size = new System.Drawing.Size(130, 40);
			this.pauseButton.TabIndex = 8;
			this.pauseButton.Text = "Pause time";
			this.pauseButton.UseVisualStyleBackColor = true;
			this.pauseButton.Click += new System.EventHandler(this.pauseButton_Click);
            // 
            // SpeedTrackBar
            // 
            this.SpeedTrackBar.AutoSize = false;
            this.SpeedTrackBar.Location = new System.Drawing.Point(11, 213);
            this.SpeedTrackBar.Maximum = 100;
			this.SpeedTrackBar.Name = "SpeedTrackBar";
            this.SpeedTrackBar.Size = new System.Drawing.Size(600, 28);
            this.SpeedTrackBar.TabIndex = 9;
            this.SpeedTrackBar.TickFrequency = 10;
            this.SpeedTrackBar.Scroll += new System.EventHandler(this.SpeedTrackBar_Scroll);
            // 
            // SpeedLabel
            // 
            this.SpeedLabel.AutoSize = true;
            this.SpeedLabel.Location = new System.Drawing.Point(260, 197);
            this.SpeedLabel.Name = "SpeedLabel";
            this.SpeedLabel.Size = new System.Drawing.Size(62, 13);
            this.SpeedLabel.TabIndex = 0;
            this.SpeedLabel.Text = "Speed = 50 %";
			// 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(650, 378);
            this.Controls.Add(this.ControlsGroupBox);
            this.Controls.Add(this.networkGroupBox);
            this.Controls.Add(this.statusStrip1);
            this.Name = "Form1";
			this.Text = "Domescape Remote";
            this.Closed += new System.EventHandler(this.MainForm_Closed);
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.networkGroupBox.ResumeLayout(false);
            this.networkGroupBox.PerformLayout();
            this.ControlsGroupBox.ResumeLayout(false);
            this.ControlsGroupBox.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.SpeedTrackBar)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
        private System.Windows.Forms.GroupBox networkGroupBox;
        private System.Windows.Forms.Label adressLabel;
        
		private System.Windows.Forms.Button connectButton;
        private System.Windows.Forms.TextBox ipTextBox;
        
		private System.Windows.Forms.Button dateButton;
		private System.Windows.Forms.TextBox dateTextBox;
		private System.Windows.Forms.Label dateLabel;

		private System.Windows.Forms.GroupBox ControlsGroupBox;
        private System.Windows.Forms.TrackBar SpeedTrackBar;
        private System.Windows.Forms.Label SpeedLabel;
		private System.Windows.Forms.Button resetButton;
		private System.Windows.Forms.Button pauseButton;
    }
}

