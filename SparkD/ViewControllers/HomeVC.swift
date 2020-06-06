//
//  HomeVC.swift
//  SparkD
//
//  Created by com on 6/3/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit

class HomeVC: UIViewController {

    @IBOutlet weak var timerContainer: UIView!
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        let titleView = navigationController?.navTitleWithImageAndText(titleText: "HOME")
        tabBarController?.navigationItem.titleView = titleView
        
        refreshTimers()
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
        timerRing.innerRingColor = UIColor.systemPink
        timerRing.outerRingWidth = 4
        timerRing.outerRingColor = UIColor.gray
        timerRing.font = UIFont.systemFont(ofSize: 12)
        timerRing.startTimer(from: from, to: 15 * 60) { (state) in
            
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
        let storyboard = UIStoryboard(name: "Main", bundle: nil)
        let vc = storyboard.instantiateViewController(withIdentifier: "ScanVC")
        tabBarController?.navigationController?.pushViewController(vc, animated: true)
    }
    
    @IBAction func btnRecentTimerViewAllTapped(_ sender: Any) {
        tabBarController?.selectedIndex = 1
    }
    
    @IBAction func btnLastReportViewAllTapped(_ sender: Any) {
        tabBarController?.selectedIndex = 2
    }
}
