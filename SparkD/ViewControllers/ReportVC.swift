//
//  ReportVC.swift
//  SparkD
//
//  Created by com on 6/3/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit
import MessageUI

class ReportVC: UIViewController {
    
    var reports = [Any]()
    
    @IBOutlet weak var tableView: UITableView!
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        tableView.dataSource = self
        tableView.delegate = self
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        let titleView = navigationController?.navTitleWithImageAndText(titleText: "REPORT")
        tabBarController?.navigationItem.titleView = titleView
        
        let exportBtn = UIBarButtonItem(title: "Export", style: .plain, target: self, action: #selector(exportBtnTapped(_:)))
        tabBarController?.navigationItem.rightBarButtonItem = exportBtn
        
        let delBtn = UIBarButtonItem(title: "Erase", style: .done, target: self, action: #selector(deleteBtnTapped(_:)))
        tabBarController?.navigationItem.leftBarButtonItem = delBtn
        
        // get reports
        if let tmp = UserDefaults.standard.value(forKey: "ReportArray") {
            reports = tmp as! [Any]
        }
        
        for i in 0..<reports.count {
            var dict = (reports[i] as! [String: Any])
            dict["selected"] = (i == 0 ? true : false)
            reports[i] = dict
        }
        
        tableView.reloadData()
    }

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

    @objc func exportBtnTapped(_ sender: Any) {
        let pdfData = NSMutableData()
        UIGraphicsBeginPDFContextToData(pdfData, view.bounds, nil)
        
        for report in reports {
            let dict = (report as! [String: Any])
            if (dict["selected"] as! Bool) == false {
                continue
            }
            
            let pdfView = UIView(frame: view.bounds)
            pdfView.backgroundColor = .white
            pdfView.layer.borderWidth = 3
            pdfView.layer.borderColor = UIColor.black.cgColor
            
            let lblTitle = UILabel(frame: CGRect(x: 20, y: 0, width: view.frame.width - 40, height: 80))
            lblTitle.text = "VITAMIN-D TEST REPORT"
            lblTitle.font = UIFont.boldSystemFont(ofSize: 23)
            pdfView.addSubview(lblTitle)
            
            let viewSep = UIView(frame: CGRect(x: 20, y: 80, width: view.frame.width - 40, height: 1))
            viewSep.backgroundColor = .black
            pdfView.addSubview(viewSep)
            
            let lblName = UILabel(frame: CGRect(x: 20, y: 85, width: view.frame.width - 40, height: 40))
            lblName.text = "TEST REPORT TITLE:     \(dict["title"] as! String)"
            lblName.font = UIFont.systemFont(ofSize: 17)
            pdfView.addSubview(lblName)
            
            let lblDate = UILabel(frame: CGRect(x: 20, y: 125, width: view.frame.width - 40, height: 40))
            lblDate.font = UIFont.systemFont(ofSize: 17)
            pdfView.addSubview(lblDate)
            
            let formatter = DateFormatter()
            formatter.dateFormat = "dd MMM, yyyy"
            let date = Date(timeIntervalSinceNow: dict["timestamp"] as! TimeInterval)
            lblDate.text = "Date of Test:    \(formatter.string(from: date))"
            
            let lblResult = UILabel(frame: CGRect(x: 20, y: 165, width: view.frame.width - 40, height: 80))
            lblResult.text = "RESULT"
            lblResult.textAlignment = .center
            lblResult.font = UIFont.boldSystemFont(ofSize: 23)
            pdfView.addSubview(lblResult)
            
            let lblValue = UILabel(frame: CGRect(x: 20, y: 245, width: view.frame.width - 40, height: 40))
            lblValue.text = "Vitamin-D (D2+D3), Total(1):     50.38 ng/ml"
            lblValue.font = UIFont.systemFont(ofSize: 15)
            pdfView.addSubview(lblValue)
            
            let lblRange = UILabel(frame: CGRect(x: 20, y: 300, width: view.frame.width - 40, height: 20))
            lblRange.text = "REFERENCE RANGE(1):"
            lblRange.font = UIFont.boldSystemFont(ofSize: 15)
            pdfView.addSubview(lblRange)
            
            let lblRangeValue = UILabel(frame: CGRect(x: 20, y: 320, width: view.frame.width - 40, height: 20))
            lblRange.text = "Desirable; 40 ng/ml - 80 ng/ml"
            lblRange.font = UIFont.systemFont(ofSize: 15)
            pdfView.addSubview(lblRangeValue)
            
            let lblNote = UILabel(frame: CGRect(x: 20, y: 370, width: view.frame.width - 40, height: 100))
            lblNote.numberOfLines = 6
            lblNote.font = UIFont.systemFont(ofSize: 13)
            lblNote.text = "Note:\n(1) Measured from SPARK-D test, not an official laboratory report\n(2)Reference range derived from general recommended from Vitamin D Council guildelines"
            pdfView.addSubview(lblNote)
            
            //self.view.addSubview(pdfView)
            UIGraphicsBeginPDFPageWithInfo(pdfView.bounds, nil)
            guard let pdfContext = UIGraphicsGetCurrentContext() else { continue }
            pdfView.layer.render(in: pdfContext)
        }
        
        UIGraphicsEndPDFContext()
        
        // save pdf to file
        let formatter = DateFormatter()
        formatter.dateFormat = "YYYYMMDDHHmmss"
        let date = Date(timeIntervalSinceNow: 0)
        
        let paths = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)
        let docDirectoryPath = paths[0]
        let pdfPath = docDirectoryPath.appendingPathComponent("\(formatter.string(from: date)).pdf")
        if pdfData.write(to: pdfPath, atomically: true) {
            print("saved pdf to \(pdfPath.absoluteString)")
        } else {
            print("error to write a pdf to  \(pdfPath.absoluteString)!")
        }
        
        // send email with pdf
        if MFMailComposeViewController.canSendMail() {
           let mail = MFMailComposeViewController()
           mail.setSubject("Report")
           mail.mailComposeDelegate = self
            
           //add attachment
            if let data = NSData(contentsOf: pdfPath) {
                mail.addAttachmentData(data as Data, mimeType: "application/pdf" , fileName: "report.pdf")
            }
 
            present(mail, animated: true)
        } else {
           print("Email cannot be sent")
        }
    }
    
    
    @objc func deleteBtnTapped(_ sender: Any) {
        var i = 0
        while i < reports.count {
            let dict = reports[i] as! [String: Any]
            if (dict["selected"] as! Bool) == true {
                reports.remove(at: i)
            } else {
                i = i + 1
            }
        }
        
        UserDefaults.standard.setValue(reports, forKey: "ReportArray")
        UserDefaults.standard.synchronize()
        
        tableView.reloadData()
    }
}

// MARK: -
extension ReportVC: MFMailComposeViewControllerDelegate {
    func mailComposeController(_ controller: MFMailComposeViewController, didFinishWith result: MFMailComposeResult, error: Error?) {
        self.dismiss(animated: true, completion: nil)
    }
}

// MARK: -
extension ReportVC: UITableViewDataSource, UITableViewDelegate {
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return reports.count
    }
    
    func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        return "Total report (\(reports.count))"
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "ReportTableViewCell", for: indexPath) as! ReportTableViewCell
        
        let report = reports[indexPath.row] as! [String: Any]
        
        cell.lblResult.text = String(format: "%.f", report["average"] as! Double)
        cell.lblName.text = report["title"] as? String
        cell.lblState.text = "Good"
        
        let formatter = DateFormatter()
        formatter.dateFormat = "dd MMM, yyyy"
        let date = Date(timeIntervalSinceNow: report["timestamp"] as! TimeInterval)
        cell.lblDate.text = formatter.string(from: date)
        
        if (report["selected"] as! Bool) == true {
            cell.accessoryType = .checkmark
        } else {
            cell.accessoryType = .none
        }
        
        return cell
    }
    
    func tableView(_ tableView: UITableView, heightForRowAt indexPath: IndexPath) -> CGFloat {
        return 90
    }
    
    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        tableView.deselectRow(at: indexPath, animated: true)
        
        var report = reports[indexPath.row] as! [String: Any]
        if (report["selected"] as! Bool) == true {
            report["selected"] = false
        } else {
            report["selected"] = true
        }
        reports[indexPath.row] = report
        
        tableView.reloadData()
    }
}
