//
//  HomeVC.swift
//  SparkD
//
//  Created by com on 6/3/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit
import SCRecorder
import Firebase
import ProgressHUD

class HomeVC: UIViewController {
    
    var recorder = SCRecorder()
    
    var capturedImageView: UIImageView?
    var btnCapture: UIButton?
    var btnNext: UIButton?
    var btnCancel: UIButton?
    var lblStep: UILabel?
    
    var captureCount = 0
    
    @IBOutlet weak var timerContainer: UIView!
    @IBOutlet weak var reportContainer: UIView!
    
    let oldSlTestingView = SL_TestingView()
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        Auth.auth().signIn(withEmail: "pritesh20@gmail.com", password: "pritesh20") { (auth, error) in
            guard error == nil else {
                print("error login to firebase with \(error!.localizedDescription)")
                return
            }
            
            print("logged in to the firebase successfully!")
        }
        
        UNUserNotificationCenter.current().delegate = self
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        let titleView = navigationController?.navTitleWithImageAndText(titleText: "HOME")
        tabBarController?.navigationItem.titleView = titleView
        
        tabBarController?.navigationItem.rightBarButtonItem = nil
        tabBarController?.navigationItem.leftBarButtonItem = nil
        
        refreshTimers()
        showLastReport()
    }
    
    func showLastReport() {
        var report = [String: Any]()
        
        guard let tmp = UserDefaults.standard.value(forKey: "ReportArray") else {
            return
        }
        
        let reports = tmp as! [Any]
        
        if reports.count == 0 {
            return
        }
        
        let cc = (reports.count > 3 ? 3 : reports.count)
        
        for index in 0..<cc {
            report = reports[index] as! [String: Any]
            
            let reportView = UIView(frame: CGRect(x: 8, y: 44 + index * 90, width: Int(view.frame.size.width) - 16, height: 70))
            reportView.backgroundColor = .white
            reportView.layer.cornerRadius = 8
            reportView.clipsToBounds = true
            reportContainer.addSubview(reportView)
            
            let resultView = UILabel(frame: CGRect(x: 8, y: 14, width: 40, height: 40))
            resultView.layer.borderWidth = 1
            resultView.layer.borderColor = UIColor.systemBlue.cgColor
            resultView.layer.cornerRadius = 4
            resultView.backgroundColor = UIColor.lightGray
            resultView.clipsToBounds = true
            resultView.text = String(format: "%.1f", report["average"] as! Double)
            resultView.textAlignment = .center
            resultView.font = UIFont.boldSystemFont(ofSize: 15)
            reportView.addSubview(resultView)
            
            let lblName = UILabel(frame: CGRect(x: 72, y: 11, width: 256, height: 21))
            lblName.text = report["title"] as? String
            lblName.font = UIFont.systemFont(ofSize: 14)
            reportView.addSubview(lblName)
            
            let lblState = UILabel(frame: CGRect(x: 72, y: 37, width: 92, height: 21))
            lblState.text = ((report["average"] as! Double) > 30 ? "Good" : "Insufficient")
            lblState.textColor = .systemBlue
            lblState.font = UIFont.systemFont(ofSize: 13)
            reportView.addSubview(lblState)
            
            let lblDate = UILabel(frame: CGRect(x: 170, y: 37, width: 167, height: 21))
            let formatter = DateFormatter()
            formatter.dateFormat = "dd MMM, yyyy"
            let date = Date(timeIntervalSinceNow: report["timestamp"] as! TimeInterval)
            lblDate.text = formatter.string(from: date)
            lblDate.textAlignment = .right
            lblDate.textColor = .gray
            lblDate.font = UIFont.systemFont(ofSize: 13)
            reportView.addSubview(lblDate)
        }
    }
    
    func refreshTimers() {
        if let array = UserDefaults.standard.value(forKey: "timers") {
            let timers = array as! [Any]
            
            checkExpiredTimer(timers: timers, index: 0)
        }
    }
    
    func checkExpiredTimer(timers: [Any], index: Int) {
        guard index < timers.count else {
            addNewTimer()
            return
        }
        
        var arrary = timers
        var curIndex = index
        let timerDict = arrary[index] as! [String: Any]
        
        let name = timerDict["name"] as! String
        
        let dateNow = Date(timeIntervalSinceNow: 0)
        let dateStart = timerDict["date"] as! Date
        let from = dateNow.timeIntervalSince(dateStart)
        
        if from > 900 {
            let alert = UIAlertController(title: nil, message: "\(name) is expired. Will you remove it?", preferredStyle: .alert)
            
            alert.addAction(UIAlertAction(title: "NO", style: .cancel, handler: { (action) in
                DispatchQueue.main.async {
                    self.checkExpiredTimer(timers: arrary, index: curIndex + 1)
                }
            }))
            
            alert.addAction(UIAlertAction(title: "YES", style: .default, handler: { (action) in
                for i in 0..<arrary.count {
                    let tmpDict = arrary[i] as! [String: Any]
                    if (tmpDict["name"] as! String) == name {
                        arrary.remove(at: i)
                        curIndex -= 1
                        break
                    }
                }
                
                UserDefaults.standard.setValue(arrary, forKey: "timers")
                UserDefaults.standard.synchronize()
                
                DispatchQueue.main.async {
                    self.checkExpiredTimer(timers: arrary, index: curIndex + 1)
                }
            }))
            self.present(alert, animated: true, completion: nil)
            
        } else {
            self.checkExpiredTimer(timers: arrary, index: curIndex + 1)
        }
    }
    
    func addNewTimer() {
        for subView in timerContainer.subviews {
            subView.removeFromSuperview()
        }
        
        if let array = UserDefaults.standard.value(forKey: "timers") {
            let timers = array as! [Any]
            
            let count = ((timers.count > 2) ? 2 : timers.count)
            for i in 0..<count {
                let timerDict = timers[i] as! [String: Any]
                let timer1 = createNewTimer(timerDict: timerDict, rt: CGRect(x: (timerContainer.frame.height + 15) * CGFloat(i),
                                                                          y: 0,
                                                                          width: timerContainer.frame.height,
                                                                          height: timerContainer.frame.height))
                timerContainer.addSubview(timer1)
            }
        }
    }
    
    func createNewTimer(timerDict: [String: Any], rt: CGRect) -> UIView {
        let timer = RoundShadowView(frame: rt)
        
        let nameLabel = UILabel(frame: CGRect(x: 0, y: timer.frame.height - 25, width: timer.frame.height, height: 25))
        nameLabel.textAlignment = .center
        nameLabel.font = UIFont.systemFont(ofSize: 14.0)
        nameLabel.text = timerDict["name"] as? String
        timer.addSubview(nameLabel)
        
        let timerRing = UICircularTimerRing(frame: CGRect(x: 15, y: 5, width: timer.frame.height - 30, height: timer.frame.height - 30))
        timer.addSubview(timerRing)
        
        let dateNow = Date(timeIntervalSinceNow: 0)
        let dateStart = timerDict["date"] as! Date
        let from = dateNow.timeIntervalSince(dateStart)
        
        timerRing.style = .ontop
        timerRing.innerRingWidth = 4
        timerRing.innerRingColor = UIColor.gray
        timerRing.outerRingWidth = 4
        timerRing.outerRingColor = UIColor.systemPink
        timerRing.startAngle = 270
        timerRing.font = UIFont.systemFont(ofSize: 12)
        timerRing.startTimer(from: from, to: 15 * 60) { (state) in
            // nothing process
        }
        
        return timer
    }

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

    @IBAction func btnStartTestTapped(_ sender: Any) {
        performSegue(withIdentifier: "segueTest", sender: nil)
    }
    
    @IBAction func btnTimerTapped(_ sender: Any) {
        if let vc = AddTimerController.addTimer() {
            vc.handler = { () in
                self.addNewTimer()
            }
            present(vc, animated: true, completion: nil)
        } else {
            let vc = UIAlertController(title: nil, message: "You can add new timer in 2 mins again", preferredStyle: .alert)
            vc.addAction(UIAlertAction(title: "OK", style: .cancel, handler: nil))
            present(vc, animated: true, completion: nil)
        }
    }
    
    @IBAction func btnScanTapped(_ sender: Any) {
        /*let storyboard = UIStoryboard(name: "Main", bundle: nil)
        let vc = storyboard.instantiateViewController(withIdentifier: "ScanVC")
        tabBarController?.navigationController?.pushViewController(vc, animated: true)*/
        showCamera()
    }
    
    @IBAction func btnRecentTimerViewAllTapped(_ sender: Any) {
        tabBarController?.selectedIndex = 1
    }
    
    @IBAction func btnLastReportViewAllTapped(_ sender: Any) {
        tabBarController?.selectedIndex = 2
    }
    
}


// MARK: -
extension HomeVC: UNUserNotificationCenterDelegate {
    func userNotificationCenter(_ center: UNUserNotificationCenter, willPresent notification: UNNotification, withCompletionHandler completionHandler: @escaping (UNNotificationPresentationOptions) -> Void) {
        completionHandler([.alert, .sound])
    }
    func userNotificationCenter(_ center: UNUserNotificationCenter, didReceive response: UNNotificationResponse, withCompletionHandler completionHandler: @escaping () -> Void) {
        if response.actionIdentifier == "scan" {
            showCamera()
        }
    }
}


// MARK: -
extension HomeVC: SCRecorderDelegate {
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
    @objc func btnCloseTapped(_ sender: Any) {
        if let subView = tabBarController?.navigationController?.view.viewWithTag(0x1000) {
            subView.removeFromSuperview()
        }
    }
    
    @objc func btnNextTapped(_ sender: Any) {
        captureCount += 1
        
        if captureCount >= 3 {
            // show report screen
            
            let alert = UIAlertController(title: nil, message: "Result Title", preferredStyle: .alert)
            
            alert.addTextField { (textField) in
                textField.placeholder = "Input Result Title"
            }
            
            alert.addAction(UIAlertAction(title: "OK", style: .default, handler: { (action) in
                var report = ReportItem()
                
                // create report data
                report.title = alert.textFields![0].text!
                
                let solver1 = GlobalHelper.getSolver1()
                let solver2 = GlobalHelper.getSolver2()
                
                report.result1 = (solver1[0] + solver2[0]) / 2
                report.result2 = (solver1[1] + solver2[1]) / 2
                report.result3 = (solver1[2] + solver2[2]) / 2
                report.result4 = (solver2[0] + solver2[1] + solver2[2]) / 3
                report.average = (report.result1 + report.result2 + report.result3 + report.result4) / 4
                
                ProgressHUD.show()
                
                // save to firebase
                let formatter = DateFormatter()
                formatter.dateFormat = "YYYYMMDDHHmmss"
                let tmpKey = formatter.string(from: report.timestamp)
                
                Database.database().reference().child("result").setValue([tmpKey: report.getDict()]) { (error, ref) in
                    ProgressHUD.dismiss()
                    
                    DispatchQueue.main.async {
                        self.btnCloseTapped(sender)
                        
                        let storyboard = UIStoryboard(name: "Main", bundle: nil)
                        let vc = storyboard.instantiateViewController(withIdentifier: "ScanVC") as! ScanVC
                        vc.reportData = report
                        self.tabBarController?.navigationController?.pushViewController(vc, animated: true)
                    }
                }
            }))
            self.present(alert, animated: true, completion: nil)
            
            return
        }
        
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
