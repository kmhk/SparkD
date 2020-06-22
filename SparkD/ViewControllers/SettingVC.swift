//
//  SettingVC.swift
//  SparkD
//
//  Created by com on 6/3/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit
import ProgressHUD

class SettingVC: UIViewController {
    
    var recorder = SCRecorder()
    
    var capturedImageView: UIImageView?
    var btnCapture: UIButton?
    var btnNext: UIButton?
    var btnCancel: UIButton?
    var lblStep: UILabel?
    
    var captureCount = 0
    
    let oldSlTestingView = SL_TestingView()

    @IBOutlet weak var switchActivePin: UISwitch!
    
    var alert: UIAlertController?
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }

    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        let titleView = navigationController?.navTitleWithImageAndText(titleText: "SETTING")
        tabBarController?.navigationItem.titleView = titleView
        
        tabBarController?.navigationItem.rightBarButtonItem = nil
        tabBarController?.navigationItem.leftBarButtonItem = nil
        
        if let isActivePin = UserDefaults.standard.value(forKey: "IsActivePin") {
            switchActivePin.isOn = isActivePin as! Bool
        } else {
            switchActivePin.isOn = true
        }
    }
    
    @objc func textFieldDidChange(_ textField: UITextField) {
        guard let alert = alert else {
            return
        }
        
        if (alert.textFields![0].text!.lengthOfBytes(using: .utf8) > 0 &&
            alert.textFields![0].text! == alert.textFields![1].text!) {
            alert.actions[1].isEnabled = true
        } else {
            alert.actions[1].isEnabled = false
        }
    }

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

    
    @IBAction func changedValueActivePin(_ sender: Any) {
        let isActivePin = switchActivePin.isOn
        
        UserDefaults.standard.setValue(isActivePin, forKey: "IsActivePin")
        UserDefaults.standard.synchronize()
    }
    
    
    @IBAction func BtnChangePinTapped(_ sender: Any) {
        alert = UIAlertController(title: nil, message: "Change Your Pin", preferredStyle: .alert)
        alert!.addTextField { (textField) in
            textField.placeholder = "Enter your pin"
            textField.keyboardType = .numberPad
            textField.addTarget(self, action: #selector(self.textFieldDidChange(_:)), for: .editingChanged)
            
        }
        alert!.addTextField { (textField) in
            textField.placeholder = "Confirm your pin"
            textField.keyboardType = .numberPad
            textField.addTarget(self, action: #selector(self.textFieldDidChange(_:)), for: .editingChanged)
        }
        
        alert!.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))
        
        let action = UIAlertAction(title: "OK", style: .default) { (action) in
            // for change pin
            let text = self.alert!.textFields![0].text!
            UserDefaults.standard.set(text, forKey: "Pin")
            UserDefaults.standard.synchronize()
            
        }
        action.isEnabled = false
        alert!.addAction(action)
        present(alert!, animated: true, completion: nil)
    }
    
    
    @IBAction func btnRepeatTestTapped(_ sender: Any) {
        UserDefaults.standard.setValue(nil, forKey: "TestMode")
        UserDefaults.standard.synchronize()
        
        showCamera()
        showAlert(0)
    }
    
    
    @IBAction func btnTestProcedureTapped(_ sender: Any) {
        performSegue(withIdentifier: "segueTest", sender: nil)
    }
    
    
    @IBAction func btnSupportTapped(_ sender: Any) {
        let actionSheet = UIAlertController(title: nil, message: "Please contact us", preferredStyle: .actionSheet)
        
        let action1 = UIAlertAction(title: "info@sparkdiagnostics.com", style: .default, handler: { (action) in
            
        })
        action1.actionImage = UIImage(named: "icoEmail")
        actionSheet.addAction(action1)
        
        let action2 = UIAlertAction(title: "+1 415 326 4177", style: .default, handler: { (action) in
            
        })
        action2.actionImage = UIImage(named: "icoPhone")
        actionSheet.addAction(action2)
        
        actionSheet.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))
        
        present(actionSheet, animated: true, completion: nil)
    }
}

// MARK: -
extension SettingVC: SCRecorderDelegate {
    func showCamera() {
        captureCount = 0
        
        let bkView = UIView(frame: (tabBarController?.navigationController?.view.bounds)!)
        bkView.backgroundColor = UIColor(red: 0, green: 0, blue: 0, alpha: 0.6)
        bkView.tag = 0x1000
        tabBarController?.navigationController?.view.addSubview(bkView)
        
        let containerView = UIView(frame: CGRect(x: 20,
                                                 y: 60,
                                                 width: bkView.frame.width - 40,
                                                 height: bkView.frame.height - 120))
        containerView.backgroundColor = UIColor.white
        containerView.layer.cornerRadius = 8
        containerView.clipsToBounds = true
        bkView.addSubview(containerView)
        
        let btnClose = UIButton(frame: CGRect(x: containerView.frame.width - 45, y: 5, width: 40, height: 40))
        btnClose.addTarget(self, action: #selector(btnCloseTapped(_:)), for: .touchUpInside)
        btnClose.setTitle("X", for: .normal)
        btnClose.setTitleColor(UIColor.darkGray, for: .normal)
        containerView.addSubview(btnClose)
        
        let lblNote = UILabel(frame: CGRect(x: 15, y: 5, width: 50, height: 40))
        lblNote.text = "Scan"
        lblNote.font = UIFont.systemFont(ofSize: 14.0)
        containerView.addSubview(lblNote)
        
        let cameraView = UIView(frame: CGRect(x: 40, y: 50, width: containerView.frame.width - 80, height: containerView.frame.height - 180))
        cameraView.backgroundColor = UIColor.black
        containerView.addSubview(cameraView)
        
        capturedImageView = UIImageView(frame: cameraView.frame)
        containerView.addSubview(capturedImageView!)
        capturedImageView!.isHidden = true
        
        initRecorder(preview: cameraView)
        
        let dashBorder = CAShapeLayer()
        dashBorder.strokeColor = UIColor.systemBlue.cgColor
        dashBorder.lineDashPattern = [3, 3]
        dashBorder.frame = cameraView.bounds
        dashBorder.fillColor = nil
        dashBorder.path = UIBezierPath(rect: cameraView.bounds).cgPath
        cameraView.layer.addSublayer(dashBorder)
        
        btnCapture = UIButton(frame: CGRect(x: cameraView.frame.midX - 60, y: containerView.frame.height - 115, width: 120, height: 36))
        btnCapture!.setAttributedTitle(NSAttributedString(string: "TAKE PICTURE",
                                                         attributes: [NSAttributedString.Key.font: UIFont.boldSystemFont(ofSize: 15),
                                                                      NSAttributedString.Key.foregroundColor: UIColor.white]), for: .normal)
        btnCapture!.backgroundColor = UIColor.darkGray
        btnCapture!.addTarget(self, action: #selector(btnTakePictureTapped(_:)), for: .touchUpInside)
        containerView.addSubview(btnCapture!)
        
        lblStep = UILabel(frame: CGRect(x: cameraView.frame.midX - 36, y: containerView.frame.height - 70, width: 72, height: 30))
        lblStep!.text = "Step 1/3"
        lblStep!.textAlignment = .center
        lblStep!.font = UIFont.systemFont(ofSize: 14.0)
        containerView.addSubview(lblStep!)
        
        btnNext = UIButton(frame: CGRect(x: cameraView.frame.midX - 90, y: containerView.frame.height - 115, width: 85, height: 36))
        btnNext!.setAttributedTitle(NSAttributedString(string: "NEXT",
                                                       attributes: [NSAttributedString.Key.font: UIFont.boldSystemFont(ofSize: 15),
                                                                    NSAttributedString.Key.foregroundColor: UIColor.white]), for: .normal)
        btnNext!.backgroundColor = UIColor.darkGray
        btnNext!.addTarget(self, action: #selector(btnNextTapped(_:)), for: .touchUpInside)
        btnNext!.isHidden = true
        containerView.addSubview(btnNext!)
        
        btnCancel = UIButton(frame: CGRect(x: cameraView.frame.midX + 5, y: containerView.frame.height - 115, width: 85, height: 36))
        btnCancel!.setAttributedTitle(NSAttributedString(string: "CANCEL",
                                                       attributes: [NSAttributedString.Key.font: UIFont.boldSystemFont(ofSize: 15),
                                                                    NSAttributedString.Key.foregroundColor: UIColor.white]), for: .normal)
        btnCancel!.backgroundColor = UIColor.darkGray
        btnCancel!.addTarget(self, action: #selector(btnCancelTapped(_:)), for: .touchUpInside)
        btnCancel!.isHidden = true
        containerView.addSubview(btnCancel!)
    }
    
    func initRecorder(preview: UIView) {
        recorder.videoConfiguration.size = preview.bounds.size
        recorder.previewView = preview
        
        recorder.captureSessionPreset = SCRecorderTools.bestCaptureSessionPresetCompatibleWithAllDevices()
        recorder.device = .back
        recorder.delegate = self
        
        recorder.session = SCRecordSession()
        recorder.session!.fileType = AVFileType.jpg.rawValue
        recorder.startRunning()
    }
    
    func detectImage(image: UIImage) {
        // process detecting image
        guard let img = self.oldSlTestingView.mainProcess(image, false, Int32(captureCount)) else { return }
        
        // show images
        capturedImageView!.image = img
        capturedImageView!.isHidden = false
        
        // show step
        btnCapture!.isHidden = true
        btnNext!.isHidden = false
        btnCancel!.isHidden = false
        
        lblStep!.text = String(format: "Step %d/3", captureCount + 1)
    }
    
    // MARK: actions
    func showAlert(_ index: Int) {
        var word = "first"
        if index == 1 {
            word = "second"
        } else if index == 2 {
            word = "third"
        }
        
        let alert = UIAlertController(title: nil, message: "Please take \(word) picture.", preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: { (action) in
            self.btnCloseTapped(self)
        }))
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler: { (action) in
            
        }))
        self.present(alert, animated: true, completion: nil)
    }
    
    @objc func btnCloseTapped(_ sender: Any) {
        if let subView = tabBarController?.navigationController?.view.viewWithTag(0x1000) {
            subView.removeFromSuperview()
        }
    }
    
    @objc func btnNextTapped(_ sender: Any) {
        var testArray = [Any]()
        if let tmp = UserDefaults.standard.value(forKey: "TestMode") {
            testArray = tmp as! [Any]
        }
        
        let res = GlobalHelper.getNResult()
        testArray.append(res as Any)
        
        UserDefaults.standard.setValue(testArray, forKey: "TestMode")
        UserDefaults.standard.synchronize()
        
        captureCount += 1
        
        if captureCount >= 3 {
            self.btnCloseTapped(sender)
            return
        }
        
        self.showAlert(captureCount)
        
        capturedImageView!.isHidden = true
        
        // show step
        btnCapture!.isHidden = false
        btnNext!.isHidden = true
        btnCancel!.isHidden = true
        
        lblStep!.text = String(format: "Step %d/3", captureCount + 1)
    }
    
    @objc func btnCancelTapped(_ sender: Any) {
        capturedImageView!.isHidden = true
        
        // show step
        btnCapture!.isHidden = false
        btnNext!.isHidden = true
        btnCancel!.isHidden = true
        
        lblStep!.text = String(format: "%d/3", captureCount + 1)
    }
    
    @objc func btnTakePictureTapped(_ sender: Any) {
        recorder.capturePhoto { (error, image) in
            guard (error == nil && image != nil) else {
                print("capture error \(error!.localizedDescription)")
                return
            }
            
            DispatchQueue.main.async {
                self.detectImage(image: image!)
            }
        }
    }
}
