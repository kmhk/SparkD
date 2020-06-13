//
//  IntroVC.swift
//  SparkD
//
//  Created by com on 6/13/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit

class IntroVC: UIViewController {

    @IBOutlet weak var collectionView: UICollectionView!
    @IBOutlet weak var btnNext: UIButton!
    @IBOutlet weak var btnSkip: UIButton!
    
    var curPage: Int = 0
    
    let strings = [
        "SPARK-D:\nTest Vitamin D with this app for instant resuls using Spark Diagnostics product",
        "SPARK-D test:\nRapid test for quantitative determination of Vitamin D in finger-prick blood samples",
        "Scan and Test:\nIntroducing Intelli-EyeQ technology for easy mobile reading",
        "Save Results:\nSave and access results anytime, and Exports results in PDF"
    ]
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        btnNext.roundSolidButton(title: "Next")
        btnNext.backgroundColor = .systemPink
        btnNext.setTitleColor(.white, for: .normal)
        
        btnSkip.setTitle("Skip", for: .normal)
        
        let layout = UICollectionViewFlowLayout()
        layout.scrollDirection = .horizontal
        collectionView.collectionViewLayout = layout
        
        collectionView.isPagingEnabled = true
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        let sz = view.frame.size
        collectionView.frame = CGRect(x: 0, y: 30, width: sz.width, height: sz.width*666/840+80)
    }
    
    @IBAction func btnNext(_ sender: Any) {
        if curPage < 3 {
            curPage += 1
            scrollToPage(curPage)
        } else {
            btnSkip(sender)
        }
    }
    
    @IBAction func btnSkip(_ sender: Any) {
        dismiss(animated: false, completion: nil)
    }
    
    func scrollToPage(_ page: Int) {
        let indexpath = collectionView.indexPathForItem(at: CGPoint(x: page * Int(collectionView.frame.size.width), y: 0))
        collectionView.scrollToItem(at: indexpath!, at: .left, animated: true)
    }

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

}

// MARK: -
extension IntroVC: UICollectionViewDataSource, UICollectionViewDelegateFlowLayout, UICollectionViewDelegate {
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return 4
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "IntroCollectionCell", for: indexPath) as! IntroCollectionCell
        
        let imageName = String(format: "intro%d", indexPath.row)
        let image = UIImage(named: imageName)
        cell.imgBack.image = image
        cell.lblNote.text = strings[indexPath.row]
        
        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAt indexPath: IndexPath) -> CGSize {
        let sz = view.frame.size
        return CGSize(width: sz.width, height: sz.width*666/840+80)
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, insetForSectionAt section: Int) -> UIEdgeInsets {
        return UIEdgeInsets(top: 0, left: 0, bottom: 0, right: 0)
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, minimumLineSpacingForSectionAt section: Int) -> CGFloat {
        return 0
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, minimumInteritemSpacingForSectionAt section: Int) -> CGFloat {
        return 0
    }
        
    func scrollViewDidScroll(_ scrollView: UIScrollView) {
        curPage = Int(scrollView.contentOffset.x / collectionView.frame.width)
    }

}
