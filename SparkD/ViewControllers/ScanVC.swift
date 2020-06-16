//
//  ScanVC.swift
//  SparkD
//
//  Created by com on 6/5/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit

class ScanVC: UIViewController {
    
    var reportData: ReportItem?
    
    var ring: UICircularProgressRing?
    
    @IBOutlet weak var btnExport: UIButton!
    @IBOutlet weak var btnSave: UIButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        let titleView = navigationController?.navTitleWithImageAndText(titleText: "RESULT")
        navigationItem.titleView = titleView
        navigationItem.hidesBackButton = true
        
        btnExport.roundSolidButton(title: "Export")
        btnExport.backgroundColor = UIColor.purple
        btnExport.setTitleColor(UIColor.white, for: .normal)
        
        btnSave.roundSolidButton(title: "Save")
        btnSave.backgroundColor = UIColor.systemPink
        btnSave.setTitleColor(UIColor.white, for: .normal)
        
        showData()
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        
        // animate ring
        ring!.startProgress(to: CGFloat(reportData!.average), duration: 1.5) {

        }
    }
    
    func showData() {
        //guard reportData != nil else {
        //    return
        //}
        
        let lblTitle = UILabel(frame: CGRect(x: view.frame.midX - 50, y: 100, width: 100, height: 35))
        lblTitle.text = reportData?.title
        lblTitle.font = UIFont.systemFont(ofSize: 21)
        lblTitle.textAlignment = .center
        view.addSubview(lblTitle)
        
        ring = UICircularProgressRing(frame: CGRect(x: view.frame.width / 6,
                                                        y: 150,
                                                        width: view.frame.width / 3 * 2,
                                                        height: view.frame.width / 3 * 2))
        view.addSubview(ring!)

        ring!.style = .ontop
        ring!.innerRingWidth = 12
        ring!.innerRingColor = UIColor.systemPink
        ring!.outerRingWidth = 4
        ring!.outerRingColor = UIColor.black
        ring!.outerCapStyle = .square
        ring!.startAngle = 144
        
        let containerView = UIView(frame: CGRect(x: 0, y: 150 + ring!.frame.height - ring!.frame.height * 0.2, width: view.frame.width, height: 100))
        containerView.backgroundColor = view.backgroundColor
        view.addSubview(containerView)
        
        let lblLow = UILabel(frame: CGRect(x: ring!.frame.minX, y: 10, width: 80, height: 30))
        lblLow.text = "Low"
        lblLow.font = UIFont.systemFont(ofSize: 13)
        lblLow.textAlignment = .center
        containerView.addSubview(lblLow)
        
        let lblHigh = UILabel(frame: CGRect(x: ring!.frame.maxX - 80, y: 10, width: 80, height: 30))
        lblHigh.text = "High"
        lblHigh.font = UIFont.systemFont(ofSize: 13)
        lblHigh.textAlignment = .center
        containerView.addSubview(lblHigh)
        
        let viewStatus = UIView(frame: CGRect(x: ring!.frame.midX - 50, y: 60, width: 100, height: 35))
        viewStatus.roundGradientView()
        containerView.addSubview(viewStatus)
        
        let lblStatus = UILabel(frame: viewStatus.bounds)
        lblStatus.font = UIFont.systemFont(ofSize: 13)
        lblStatus.textAlignment = .center
        lblStatus.text = (reportData!.average > 30 ? "Good" : "Insufficient")
        lblStatus.textColor = .white
        viewStatus.addSubview(lblStatus)
    }
    
    @IBAction func btnExportTapped(_ sender: Any) {
        navigationController?.popViewController(animated: true)
    }
    
    @IBAction func btnSaveTapped(_ sender: Any) {

        var reportArray = [Any]()
        
        if let tmp = UserDefaults.standard.value(forKey: "ReportArray") {
            reportArray = tmp as! [Any]
        }
        
        //reportArray.append(reportData!.getDict())
        reportArray.insert(reportData!.getDict(), at: 0)
        
        UserDefaults.standard.setValue(reportArray, forKey: "ReportArray")
        UserDefaults.standard.synchronize()
        
        let alert = UIAlertController(title: nil, message: "Report saved successfully", preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .cancel, handler: { (action) in
            self.navigationController?.popViewController(animated: true)
        }))
        present(alert, animated: true, completion: nil)
    }
}
